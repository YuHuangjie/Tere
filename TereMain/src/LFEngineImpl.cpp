#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <thread>

#include "Config.h"
#include "common/Log.hpp"
#include "LinearUI.h"
#include "ArcballUI.h"
#include "Strategy.h"
#include "Interpolation.h"

#include "LFEngineImpl.h"
#include "Error.h"
#include "Renderer.h"
#include "TextureFuser.h"
#include "Poster.h"
#include "TereScene.h"
#include "WeightedCamera.h"
#include "image/Image.hpp"

using namespace std;

static bool DecodeImage(const string &file, const DecHeaderFunc &fdh, 
	const DecImageFunc &fdi, shared_ptr<uint8_t> &image, int &width, int &height,
	const float zoom = 1.f)
{
	// decode image header
	int w = 0, h = 0;

	if (!fdh(file.c_str(), &w, &h)) {
		RETURN_ON_ERROR("DecHeaderFunc returned false");
	}

	// decode image
	const int newW = w * zoom;
	const int newH = h * zoom;
	const size_t size = newW * newH * 3;
	shared_ptr<uint8_t> _image(new uint8_t[size], [](uint8_t *p) { delete[] p; });

	if (!fdi(file.c_str(), newW, newH, _image.get(), size)) {
		RETURN_ON_ERROR("DecImageFunc returned false");
	}

	width = newW;
	height = newH;
	image = _image;

	return true;
}

static float AverageNorm(const vector<Extrinsic> &cameras)
{
	const size_t N = cameras.size();
	vector<float> nnDist(N);

	if (N == 0) { return 0.f; }

	// calculate nearest neighbor for every cameras
	for (size_t i = 0; i < N; ++i) {
		const glm::vec3 &myPos = cameras[i].Pos();
		float minDist = std::numeric_limits<float>::max();

		for (size_t j = 0; j < N; ++j) {
			if (j == i) continue;
			const glm::vec3 &pos = cameras[j].Pos();
			const glm::vec3 &diff = myPos - pos;
			float dist = glm::dot(diff, diff);
			if (dist < minDist) { minDist = dist; }
		}

		nnDist[i] = minDist;
	}

	// compute average distance
	float average = std::accumulate(nnDist.cbegin(), nnDist.cend(), 0.f) / N;
	return average;
}

static float CalculateSlotsMultiplier(const vector<Extrinsic> &extrinsics)
{
	// Maximum slots between i-th ref camera and its NN j-th ref camera.
	const float HARD_CODED_INTERVAL = 15.f;

	return HARD_CODED_INTERVAL / AverageNorm(extrinsics);
}

LFEngineImpl::LFEngineImpl(const size_t nCams, const RENDER_MODE mode)
	: 
	_mode(INTERP),
	_scene(nullptr),
	fdh(nullptr),
	fdi(nullptr),
	_renderer(nullptr),
	_textureFuser(nullptr),
	_poster(nullptr),
	_renderCam(),
	_UI(nullptr),
	_fixRef(0),
	_fps(0),
	_frames(0),
	_schStrg(nullptr),
	_wghStrg(nullptr),
	_locked(false)
{
	if (nCams == 0) {
		THROW_ON_ERROR("Invalid nCams");
	}
	if (mode != LINEAR && mode != SPHERE && mode != ALL) {
		THROW_ON_ERROR("Invalid mode");
	}

	_scene.reset(new TereScene(nCams));
	_scene->rmode = mode;

	if (nCams > 12 && mode == ALL) {
		LOGW("[WARNING] LFEngine: No. cameras is too large. Try not use ALL mode\n");
	}
}

LFEngineImpl::~LFEngineImpl(void)
{
	_scene = nullptr;
	_renderer = nullptr;
	_textureFuser = nullptr;
	_poster = nullptr;
	_UI = nullptr;
}

bool LFEngineImpl::SetGeometry(const float * v, const size_t szV, bool GPU)
{
	if (!v) {
		RETURN_ON_ERROR("v is NULL");
	}

	return _scene->UpdateGeometry(v, szV, nullptr, 0, GPU);
}

bool LFEngineImpl::SetGeometry(const float *v, const size_t szV, const int *f,
	const size_t szF, bool GPU)
{
	if (!v) { 
		RETURN_ON_ERROR("v is NULL");
	}
	if (!f) {
		RETURN_ON_ERROR("f is NULL");
	}

	return _scene->UpdateGeometry(v, szV, f, szF, GPU);
}

#define CHECK_ID(id, max) do {\
	if (id < 0 || id >= max) RETURN_ON_ERROR("Invalid camera index");\
} while (0)

bool LFEngineImpl::SetRefImage(const size_t id, const uint8_t *rgb, const size_t w,
	const size_t h)
{
	CHECK_ID(id, _scene->nCams);

	return _scene->UpdateImage(id, rgb, w, h);
}

bool LFEngineImpl::SetRefImage(const size_t id, const string & filename,
	const float zoom)
{
	CHECK_ID(id, _scene->nCams);

	if (filename.empty()) {
		RETURN_ON_ERROR("filename is empty");
	}
	if (zoom < 1e-3) {
		RETURN_ON_ERROR("Too small zooming factor");
	}
	if (!fdh || !fdi) {
		RETURN_ON_ERROR("Image decoding functions are not registered properly");
	}

	int width = 0, height = 0;
	shared_ptr<uint8_t> image = nullptr;

	if (!DecodeImage(filename, fdh, fdi, image, width, height, zoom)) {
		RETURN_ON_ERROR("Decode image failed");
	}

	if (!_scene->UpdateImage(id, image.get(), width, height)) {
		RETURN_ON_ERROR("Scene update image failed");
	}

	return true;
}

bool LFEngineImpl::SetCamera(const size_t id, const array<float, 9> &K, 
	const array<float, 16> &M, bool w2c, bool yIsUp)
{
	CHECK_ID(id, _scene->nCams);

	return _scene->UpdateCamera(id, K, M, w2c, yIsUp);
}

void LFEngineImpl::SetRows(const size_t rows)
{
	_scene->rows = rows;
}

void LFEngineImpl::RegisterDecFunc(const DecHeaderFunc _fdh, const DecImageFunc _fdi)
{
	fdh = _fdh;
	fdi = _fdi;
}

bool LFEngineImpl::HaveSetScene()
{
	if (!_scene->Configure()) return false;

	try {
		// Initialize scene renderer
		LOGI("ENGINE: preparing scene renderer\n");
		_renderer.reset(new Renderer(_scene));
		_renderer->UpdatedGeometry();
		_renderer->UpdatedLF();

		// initialize texture fuser
		LOGI("ENGINE: preparing TextureFuser\n");
		_textureFuser.reset(new TextureFuser(_scene->width, _scene->height,
			0.f, 0.f, 0.f));

		// initialize poster for screen rendering
		LOGI("ENGINE: preparing Poster\n");
		_poster.reset(new Poster());

		LOGI("ENGINE: setting others\n");
		
		// initialize viewport sizes
		_offlineViewport = vector<int>{ 0, 0, _scene->width, _scene->height };
		_screenViewport = vector<int>{ 0, 0, _scene->width, _scene->height };

		// rendering camera, user interface and interpolation strategies settings
		// are dependent on different rendering mode
		switch (_scene->rmode)
		{
		case RENDER_MODE::LINEAR:
			_renderCam = Camera({ _scene->intrins[0], _scene->extrins[0] });
			_schStrg.reset(new DefaultSearchStrategy(_scene->center));
			_UI.reset(new LinearUI(_scene->extrins, _scene->rows, 0, _scene->width, _scene->height));
			break;
		case RENDER_MODE::ALL:
			_renderCam = Camera({ _scene->intrins[0], Extrinsic(
				&(_scene->extrins[0].Pos())[0], 
				&(_scene->center)[0],
				&(_scene->extrins[0].Up())[0]) });
			_schStrg.reset(new AllSearchStrategy);
			_UI.reset(new ArcballUI(_scene->width, _scene->height, _scene->center));
			break;
		case RENDER_MODE::SPHERE: 
			_renderCam = Camera({ _scene->intrins[0], Extrinsic(
				&(_scene->extrins[0].Pos())[0],
				&(_scene->center)[0],
				&(_scene->extrins[0].Up())[0]) });
			_schStrg.reset(new DefaultSearchStrategy(_scene->center));
			_UI.reset(new ArcballUI(_scene->width, _scene->height, _scene->center));
			break;
		default:
			RETURN_ON_ERROR("Unknown render mode");
			break;
		}

		// weighing strategy is default
		_wghStrg.reset(new DefaultWeighStrategy(_scene->center));

		// (in)decrease focal length to simulate zooming effects
		_stdFx = _renderCam.intrin.fx;
		_stdFy = _renderCam.intrin.fy;

		// compute intervals per distance
		_SLOT_MULTIPLIER = CalculateSlotsMultiplier(_scene->extrins);
	}
	catch (std::exception &e) {
		THROW_ON_ERROR(e.what());
	}

	return true;
}

bool LFEngineImpl::HaveUpdatedScene()
{
	if (!_scene->Configure()) {
		return false;
	}

	if (_renderer) {
		_renderer->UpdatedGeometry();
		_renderer->UpdatedLF();
	}
	return true;
}

void LFEngineImpl::Draw(void)
{
	if (_locked) {
		_renderCam.extrin = _slotQueue.front();
		_slotQueue.pop_front();
		_locked = !_slotQueue.empty();
	}

	_Draw();
}

void LFEngineImpl::_Draw(void)
{
	switch (_mode) {
	case INTERP: {
		_renderer->SetViewer(glm::mat4(1.f), _renderCam.extrin.viewMat,
			_renderCam.intrin.ProjMat(_scene->glnear, _scene->glfar,
				_scene->width, _scene->height));

		// search interpolation cameas and calculate weights
		size_t nInterp = 12;
		vector<size_t> indices;
		vector<float> weights;

		if (_schStrg) {
			indices = _schStrg->Search(_scene->extrins, _renderCam.extrin, nInterp);
		}
		else {
			indices = _UI->HintInterp();
		}
		
		weights = _wghStrg->Weigh(_scene->extrins, _renderCam.extrin, indices);
		nInterp = std::min(indices.size(), weights.size());

		_renderer->ClearInterpCameras();

		for (size_t i = 0; i < indices.size(); ++i) {
			_renderer->AddInterpCameras(WeightedCamera(indices[i], weights[i]));
		}

		break;
	}
	case FIX: {
		_renderer->SetViewer(glm::mat4(1.f), _scene->extrins[_fixRef].viewMat,
			_scene->intrins[_fixRef].ProjMat(_scene->glnear, _scene->glfar,
				_scene->width, _scene->height));

		// select a fixed camera for interpolation
		_renderer->ClearInterpCameras();
		_renderer->AddInterpCameras(WeightedCamera(
			static_cast<int>(_fixRef), 1.0));
		break;
	}
	default:break;
	}

	// 1st pass: scene rendering
	unsigned int renderTex = _renderer->Render(_offlineViewport);
	
	// 2nd pass: background fusion
	_textureFuser->SetForeground(renderTex);
	unsigned int fusedTex = _textureFuser->Render(_offlineViewport);

	// 3rd pass: render to screen 
	_poster->SetTexture(fusedTex);
	_poster->Render(_screenViewport);
	++_frames;
}

void LFEngineImpl::StartFPSThread(void)
{
	std::thread t(&LFEngineImpl::VRFPS, this);
	t.detach();
}

void LFEngineImpl::VRFPS(void)
{
	while (_renderer)
	{
		this_thread::sleep_for(chrono::seconds(1));
		_fps = _frames;
		_frames = 0;
	}
}

void LFEngineImpl::Resize(uint32_t width, uint32_t height)
{
	LOGD("Engine: screen: width: %d  height: %d\n", width, height);

	float aspect = glm::max(static_cast<float>(_scene->width) / width,
		static_cast<float>(_scene->height) / height);

	_screenViewport.clear();
	_screenViewport.push_back(width / 2 - _scene->width / aspect / 2);
	_screenViewport.push_back(height / 2 - _scene->height / aspect / 2);
	_screenViewport.push_back(_scene->width / aspect);
	_screenViewport.push_back(_scene->height / aspect);

	_UI->SetResolution(width, height);
}

void LFEngineImpl::SetUI(UIType type, float sx, float sy)
{
	if (_locked) {
		return;
	}

	switch (type) {
	case MOVE:
		_renderCam.extrin = _UI->Move(sx, sy, _renderCam.extrin);
		break;
	case TOUCH:
		_UI->Touch(sx, sy);
		_mode = INTERP;
		break;
	case LEAVE: {
		Extrinsic dst = _UI->Leave(sx, sy, _renderCam.extrin);

		/* Move to the nearest reference camera */
		EnqueueSlots(_renderCam.extrin, dst);
		_locked = !_slotQueue.empty();

		/* Stay where it is */
		// _renderCam.extrin = dst;
		// _locked = false;
		break;
	}
	default:
		throw runtime_error("Setting false UI type");
		break;
	}
}

void LFEngineImpl::EnqueueSlots(const Extrinsic &start, const Extrinsic &end)
{
	_slotQueue.clear();

	const glm::vec3 diff = start.Pos() - end.Pos();
	const size_t intervals = static_cast<size_t>(std::ceil(
		glm::dot(diff, diff) * _SLOT_MULTIPLIER));

	if (intervals <= 0) { return; }

	for (int i = 1; i <= intervals; ++i) {
		const Extrinsic e = Interp(start, end, static_cast<float>(i) / intervals);
		_slotQueue.push_back(e);
	}
}

void LFEngineImpl::SetLocationOfReferenceCamera(int id)
{
	_fixRef = id;
	_mode = FIX;
}

float LFEngineImpl::SetZoomScale(float zoom_scale)
{
    if (zoom_scale <= 0.1f) {
        zoom_scale = 0.1f;
    }
    else if (zoom_scale >= 10.f) {
        zoom_scale = 10.f;
    }

	_renderCam.intrin.fx = _stdFx * zoom_scale;
	_renderCam.intrin.fy = _stdFy * zoom_scale;

    return zoom_scale;
}

bool LFEngineImpl::GetScreenShot(unsigned char *buffer, int x, int y, int width, int height) const
{
	if (buffer == nullptr) {
		return false;
	}
	if (x < 0 || y < 0 || width <= 0 || height <= 0) {
		return false;
	}
	if (glGetError() != 0) {
		return false;
	}

	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	if (glGetError() != 0) {
		return false;
	}
	return true;
}

bool LFEngineImpl::SetBackground(float r, float g, float b)
{
	if (!_textureFuser) {
		return false;
	}

	float _r = std::max<float>(std::min<float>(r, 1.0f), 0.0f);
	float _g = std::max<float>(std::min<float>(g, 1.0f), 0.0f);
	float _b = std::max<float>(std::min<float>(b, 1.0f), 0.0f);

	_textureFuser->SetBackground(_r, _g, _b);
	
	return true;
}

bool LFEngineImpl::SetBackground(const string &imagePath)
{
	if (!_textureFuser) {
		RETURN_ON_ERROR("texture fuser is NULL");
	}
	if (!fdi || !fdh) {
		RETURN_ON_ERROR("Decoding functions are not registered");
	}

	shared_ptr<uint8_t> image = nullptr;
	int width = 0, height = 0;

	if (!DecodeImage(imagePath, fdh, fdi, image, width, height)) {
		RETURN_ON_ERROR("Decode background failed");
	}

	if (!_textureFuser->SetBackground(Image(image, width, height, 3))) {
		RETURN_ON_ERROR("Set background failed");
	}

	return true;
}

void LFEngineImpl::SetScreenFBO(unsigned int fbo)
{
    if (_poster) { _poster->SetScreenFBO(fbo); }
}
