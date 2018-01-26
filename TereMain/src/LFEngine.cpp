#include "LFEngine.h"
#include "common/Log.hpp"
#include <chrono>
#include <cstdlib>

using std::chrono::seconds;
using std::this_thread::sleep_for;

LFEngine::LFEngine(const string &profile)
	: gLFLoader(nullptr),
	gOBJRender(nullptr),
	gRenderCamera(),
	fps(0),
	frames(0),
	ui(nullptr)
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

		glm::vec3 look_center = attrib.ref_camera_center;
		const float render_cam_r = attrib.ref_camera_radius;
		glm::vec3 location = glm::vec3(render_cam_r*sin(glm::pi<float>() / 2)*cos(0),
			render_cam_r*sin(glm::pi<float>() / 2)*sin(0),
			render_cam_r*cos(glm::pi<float>() / 2)) + look_center;
		glm::vec3 _up = glm::vec3(0, 0, 1);
		glm::vec3 lookat = (look_center - location) / glm::length(look_center - location);
		glm::vec3 right = glm::cross(lookat, _up);
		glm::vec3 up = glm::cross(right, lookat);
		up = glm::normalize(up);

		gRenderCamera.SetExtrinsic(Extrinsic(location, look_center, up));
		gRenderCamera.SetIntrinsic(Intrinsic(
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetCx(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetCy(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetFx(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetFy()));

		// Set up user interface
		ui = new UserInterface(0, 0, up, look_center);
	}
	catch (runtime_error &e) {
		LOGW("runtime error occured: %s\n", e.what());
		std::abort();
	}

	LOGI("\n===============  ENGINE INITIALIZATION OK!  ================\n");
}

void LFEngine::Draw(void)
{
	gOBJRender->SetVirtualCamera(gRenderCamera);
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

void LFEngine::SetUI(int type, double sx, double sy)
{
	switch (type) {
	case 0:
		ui->FingerMove(sx, sy, gRenderCamera);
		break;
	case 1:
		ui->FingerDown(true, sx, sy);
		gOBJRender->UseHighTexture(false);
		break;
	case 2:
		ui->FingerDown(false, sx, sy);
		gOBJRender->UseHighTexture(true);
        gOBJRender->ReplaceHighTexture();
		break;
	default:
		throw runtime_error("Setting false UI type");
		break;
	}
}

void LFEngine::SetLocationOfReferenceCamera(int id)
{
	if (id < 0 || id >= gLFLoader->GetLightFieldAttrib().N_REF_CAMERAS) {
		return;
	}

	gRenderCamera = gLFLoader->GetLightFieldAttrib().ref_cameras[id];
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
