#include "LFEngine.h"
#include "common/Log.hpp"
#include "CircleUI.h"
#include "LinearUI.h"
#include "WeightedCamera.h"
#include "InterpStrategy.h"
#include <chrono>
#include <cstdlib>

using std::chrono::seconds;
using std::this_thread::sleep_for;

LFEngine::LFEngine(const string &profile)
	: _mode(INTERP),
	_fixRef(0),
	gLFLoader(nullptr),
	gOBJRender(nullptr),
	gRenderCamera(),
	fps(0),
	frames(0),
	ui(nullptr),
	_interpStrgFunc(DefaultInterpStrgFunc)
{
	InitEngine(profile);
}

LFEngine::~LFEngine(void)
{
	gLFLoader.reset(nullptr);
	gOBJRender.reset(nullptr);
	delete ui;
	ui = nullptr;
}

void LFEngine::InitEngine(const string &profile)
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
		gOBJRender.reset(new OBJRender(attrib, attrib.width_H, attrib.height_H));

		// Read light field textures
		LOGI("ENGINE: decompressing images\n");
		gLFLoader->Decompress(4);
		LOGI("ENGINE: decompressing done\n");

		// Append depth channel to each texture
		LOGI("ENGINE: generating RGBD textures\n");
		vector<GLuint> rgbds = gLFLoader->GenerateRGBDTextures(gOBJRender);

		// Transfer light field textures
		gOBJRender->SetLightFieldTexs(rgbds);
		//glDeleteTextures(rgbs.size(), rgbs.data());

		// Set rendering camera
		LOGI("ENGINE: setting rendering camera\n");

		glm::vec3 center = attrib.ref_camera_center;
		const float r = attrib.ref_camera_radius;
		glm::vec3 up = attrib.ref_camera_up;
		glm::vec3 location = glm::vec3(center.x,
			center.y - r * up.z / std::sqrt(up.y*up.y + up.z*up.z),
			center.z + r * up.y / std::sqrt(up.y*up.y + up.z*up.z));


		
		gRenderCamera.SetIntrinsic(attrib.ref_cameras[0].GetIntrinsic());

		// Set up user interface
		// 
		// Circle UI
		//gRenderCamera.SetExtrinsic(Extrinsic(location, center, up));
		//ui = new CircleUI(0, up, center);

		// Linear UI
		gRenderCamera.SetExtrinsic(attrib.ref_cameras[0].GetExtrinsic());
		ui = new LinearUI(0, attrib.ref_cameras, 0.f);

		// arcball UI
		/*glm::vec3 location = glm::vec3(render_cam_r*sin(glm::pi<float>() / 2)*cos(0),
		render_cam_r*sin(glm::pi<float>() / 2)*sin(0),
		render_cam_r*cos(glm::pi<float>() / 2)) + look_center;
		glm::vec3 _up = glm::vec3(0, 0, 1);
		glm::vec3 lookat = (look_center - location) / glm::length(look_center - location);
		glm::vec3 right = glm::cross(lookat, _up);
		glm::vec3 up = glm::cross(right, lookat);
		up = glm::normalize(up);*/
		//ui = new ArcballUI(0, 0, up, look_center);
	}
	catch (runtime_error &e) {
		LOGW("runtime error occured: %s\n", e.what());
		std::abort();
	}

	LOGI("\n===============  ENGINE INITIALIZATION OK!  ================\n");
}

void LFEngine::Draw(void)
{
	switch (_mode) {
	case INTERP: {
		gOBJRender->SetVirtualCamera(gRenderCamera);

		// select which cameas and weights to interpolate
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();
		gOBJRender->SetInterpCameras(_interpStrgFunc(
			attrib.ref_cameras, gRenderCamera, 
			attrib.ref_camera_center, 3));

		break;
	}
	case FIX: {
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();
		gOBJRender->SetVirtualCamera(attrib.ref_cameras[_fixRef]);

		// select a fixed camera for interpolation
		gOBJRender->SetInterpCameras(WeightedCamera(
			static_cast<int>(_fixRef), 1.0));
		break;
	}
	default:break;
	}	

	// call underlying renderer
	gOBJRender->render(viewport);
	++frames;
}

void LFEngine::StartFPSThread(void)
{
	std::thread t(&LFEngine::VRFPS, this);
	t.detach();
}

void LFEngine::VRFPS(void)
{
	while (gOBJRender)
	{
		sleep_for(seconds(1));
		fps = frames;
		frames = 0;
	}
}

void LFEngine::Resize(GLuint width, GLuint height)
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

void LFEngine::SetUI(UIType type, double sx, double sy)
{
	switch (type) {
	case MOVE:
		gRenderCamera = ui->Move(sx, sy, gRenderCamera);
		break;
	case TOUCH:
		ui->Touch(sx, sy);
		gOBJRender->UseHighTexture(false);     
		_mode = INTERP;
		break;
	case LEAVE:
		gRenderCamera = ui->Leave(sx, sy, gRenderCamera);
		//gOBJRender->SetVirtualCamera(gRenderCamera);
        //gOBJRender->ReplaceHighTexture();
		//gOBJRender->UseHighTexture(true);
		break;
	default:
		throw runtime_error("Setting false UI type");
		break;
	}
}

void LFEngine::SetLocationOfReferenceCamera(int id)
{
	_fixRef = id;
	_mode = FIX;
}

void LFEngine::SetZoomScale(float zoom_scale)
{
	if (zoom_scale <= 0.0f) {
		return;
	}

	glm::vec3 new_position = gRenderCamera.GetPosition() - (zoom_scale - 1.0f) * gRenderCamera.GetDir();
	gRenderCamera.SetExtrinsic(Extrinsic(new_position, 
		gRenderCamera.GetPosition()-gRenderCamera.GetDir(), 
		gRenderCamera.GetUp()));
}

bool LFEngine::GetScreenShot(unsigned char *buffer, int x, int y, int width, int height)
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
