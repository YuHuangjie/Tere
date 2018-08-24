#include <chrono>
#include <cstdlib>
#include <algorithm>

#include "LFEngineImpl.h"
#include "common/Log.hpp"
#include "LinearUI.h"
#include "ArcballUI.h"
#include "WeightedCamera.h"
#include "Strategy.h"

using std::chrono::seconds;
using std::this_thread::sleep_for;

LFEngineImpl::LFEngineImpl(const string &profile)
	: _mode(INTERP),
	_fixRef(0),
	gLFLoader(nullptr),
	gRenderer(nullptr),
	gRenderCamera(),
	fps(0),
	frames(0),
	ui(nullptr),
	_indexStrgFunc(DefaultIndexStrgFunc),
	_weightStrgFunc(DefaultWeightStrgFunc)
{
	InitEngine(profile);
}

LFEngineImpl::~LFEngineImpl(void)
{
	gLFLoader.reset(nullptr);
	gRenderer.reset(nullptr);
	delete ui;
	ui = nullptr;
}

void LFEngineImpl::InitEngine(const string &profile)
{
	if (profile.empty()) {
		throw std::runtime_error("invalid profile file");
	}

	try {
		// Initialize light field loader
		gLFLoader.reset(new LFLoader(profile));

		// Light field configuaration
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();

		// Initialize obj renderer
		LOGI("ENGINE: preparing Renderer\n");
		gRenderer.reset(new Renderer(attrib, attrib.width_H, attrib.height_H));

		// Read light field textures
		LOGI("ENGINE: decompressing images\n");
		gLFLoader->Decompress(4);
		LOGI("ENGINE: decompressing done\n");

		// Append depth channel to each texture
		LOGI("ENGINE: generating RGBD textures\n");
		vector<GLuint> rgbds = gLFLoader->GenerateRGBDTextures(gRenderer);

		// Transfer light field textures
		gRenderer->SetLightFieldTexs(rgbds);

		// Set rendering camera
		LOGI("ENGINE: setting rendering camera\n");
		
		gRenderCamera.SetIntrinsic(attrib.ref_cameras[0].GetIntrinsic());

		// Set up user interface
		if (attrib._uiMode == "linear") {
			// Linear UI
			gRenderCamera.SetExtrinsic(attrib.ref_cameras[0].GetExtrinsic());
			ui = new LinearUI(attrib.ref_cameras, attrib._rows, 0);
		}
		else if (attrib._uiMode == "arcball") {
			// arcball UI
			float r = attrib.ref_camera_radius;
			glm::vec3 center = attrib.ref_camera_center;

			glm::vec3 location = glm::vec3(r*sin(glm::pi<float>() / 2)*cos(0),
				r*sin(glm::pi<float>() / 2)*sin(0),	
				r*cos(glm::pi<float>() / 2) ) + center;
			glm::vec3 _up = glm::vec3(0, 0, 1);
			glm::vec3 lookat = glm::normalize(center - location);
			glm::vec3 right = glm::cross(lookat, _up);
			glm::vec3 up = glm::cross(right, lookat);

			gRenderCamera.SetExtrinsic(Extrinsic(location, center, up));
			ui = new ArcballUI(up, center);
		}
	}
	catch (runtime_error &e) {
		LOGW("runtime error occured: %s\n", e.what());
		std::abort();
	}

	LOGI("\n===============  ENGINE INITIALIZATION OK!  ================\n");
}

void LFEngineImpl::Draw(void)
{
	switch (_mode) {
	case INTERP: {
		gRenderer->SetVirtualCamera(gRenderCamera);

		// select which cameas and weights to interpolate
		size_t nInterp = 3;
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();
		vector<size_t> indices;
		if (ui->Name() == "linear") {
			indices = static_cast<LinearUI*>(ui)->HintInterp();
		}
		else {
			indices = _indexStrgFunc(attrib.ref_cameras,
				gRenderCamera, attrib.ref_camera_center, nInterp);
		}
		vector<float> weights = _weightStrgFunc(attrib.ref_cameras,
			gRenderCamera, attrib.ref_camera_center, indices);
		nInterp = std::min(indices.size(), weights.size());

		gRenderer->ClearInterpCameras();
		for (size_t i = 0; i < indices.size(); ++i) {
			gRenderer->AddInterpCameras(WeightedCamera(indices[i], weights[i]));
		}

		break;
	}
	case FIX: {
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();
		gRenderer->SetVirtualCamera(attrib.ref_cameras[_fixRef]);

		// select a fixed camera for interpolation
		gRenderer->ClearInterpCameras();
		gRenderer->AddInterpCameras(WeightedCamera(
			static_cast<int>(_fixRef), 1.0));
		break;
	}
	default:break;
	}

	// call underlying renderer
	gRenderer->render(viewport);
	++frames;
}

void LFEngineImpl::StartFPSThread(void)
{
	std::thread t(&LFEngineImpl::VRFPS, this);
	t.detach();
}

void LFEngineImpl::VRFPS(void)
{
	while (gRenderer)
	{
		sleep_for(seconds(1));
		fps = frames;
		frames = 0;
	}
}

void LFEngineImpl::Resize(uint32_t width, uint32_t height)
{
	LOGD("Engine: screen: width: %d  height: %d\n", width, height);

	LightFieldAttrib attrib = gLFLoader->GetLightFieldAttrib();
	float aspect = glm::max(static_cast<float>(attrib.width_H) / width,
		static_cast<float>(attrib.height_H) / height);

	viewport.clear();
	viewport.push_back(width / 2 - attrib.width_H / aspect / 2);
	viewport.push_back(height / 2 - attrib.height_H / aspect / 2);
	viewport.push_back(attrib.width_H / aspect);
	viewport.push_back(attrib.height_H / aspect);

	ui->SetResolution(width, height);
}

void LFEngineImpl::SetUI(UIType type, double sx, double sy)
{
	switch (type) {
	case MOVE:
		gRenderCamera = ui->Move(sx, sy, gRenderCamera);
		break;
	case TOUCH:
		ui->Touch(sx, sy);
		gRenderer->UseHighTexture(false);
		_mode = INTERP;
		break;
	case LEAVE:
		gRenderCamera = ui->Leave(sx, sy, gRenderCamera);
		//gRenderer->SetVirtualCamera(gRenderCamera);
		//gRenderer->ReplaceHighTexture();
		//gRenderer->UseHighTexture(true);
		break;
	default:
		throw runtime_error("Setting false UI type");
		break;
	}
}

void LFEngineImpl::SetLocationOfReferenceCamera(int id)
{
	_fixRef = id;
	_mode = FIX;
}

void LFEngineImpl::SetZoomScale(float zoom_scale)
{
	if (zoom_scale <= 0.0f) {
		return;
	}

	glm::vec3 new_position = gRenderCamera.GetPosition() - (zoom_scale - 1.0f) * gRenderCamera.GetDir();
	gRenderCamera.SetExtrinsic(Extrinsic(new_position,
		gRenderCamera.GetPosition() - gRenderCamera.GetDir(),
		gRenderCamera.GetUp()));
}

bool LFEngineImpl::GetScreenShot(unsigned char *buffer, int x, int y, int width, int height)
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
