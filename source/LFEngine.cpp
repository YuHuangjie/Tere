#include "LFEngine.h"
#include <chrono>
#include <cstdlib>

LFEngine::LFEngine(const string &profile)
{
	InitEngine(profile);
	fps = 0;
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
	try {
		// initialize light field loader
		gLFLoader.reset(new LFLoader(profile));

		// fetch light field configuaration
		const LightFieldAttrib &attrib = gLFLoader->GetLightFieldAttrib();

		// initialize obj renderer
		LOGI("ENGINE: preparing Renderer\n");
		gOBJRender.reset(new OBJRender(attrib.obj_file,
			attrib.width_H, attrib.height_H,
			attrib.camera_mesh_name));

		// set light field configuaration
		LOGI("ENGINE: setting light field configuaration\n");
		gOBJRender->SetLightFieldAttrib(attrib);

		// read light field
		LOGI("ENGINE: decompressing images\n");
		gLFLoader->PreDecompress();
		gLFLoader->Decompress(4);
		LOGI("ENGINE: decompressing done\n");
		gLFLoader->PostDecompress();
		LOGI("ENGINE: generating RGBD textures\n");
		gLFLoader->GenerateRGBDTexture(gOBJRender);

		// resetting light field configuration (light_field_tex changed)
		LOGI("ENGINE: resetting light field configuaration\n");
		gOBJRender->SetLightFieldAttrib(attrib);

		// set rendering camera
		LOGI("ENGINE: setting rendering camera\n");
		glm::vec3 look_center = attrib.ref_camera_center;

		float render_cam_r = gLFLoader->GetRefCameraRadius();
		glm::vec3 location = glm::vec3(render_cam_r*sin(glm::pi<float>() / 2)*cos(0),
			render_cam_r*sin(glm::pi<float>() / 2)*sin(0),
			render_cam_r*cos(glm::pi<float>() / 2)) + look_center;
		glm::vec3 _up = glm::vec3(0, 0, 1);
		glm::vec3 lookat = (look_center - location) / glm::length(look_center - location);
		glm::vec3 right = glm::cross(lookat, _up);
		glm::vec3 up = glm::cross(right, lookat);
		lookat = glm::normalize(lookat);
		up = glm::normalize(up);
		gRenderCamera.setParameter(location, lookat, up,
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetFOVW(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetFOVH(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetCx(),
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetCy(), 
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetWidth(), 
			gLFLoader->GetLightFieldAttrib().ref_cameras[0].GetHeight());

		// set up user interface
		ui = new UserInterface(0, 0, up, look_center);
	}
	catch (runtime_error &e) {
		LOGW("runtime error occured: %s\n", e.what());
		exit(1);
	}

	LOGI("\n===============  ENGINE INITIALIZATION OK!  ================\n");
}

void LFEngine::Draw(void)
{
	gOBJRender->SetVirtualCamera(gRenderCamera);
	gOBJRender->render();
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
		std::this_thread::sleep_for(chrono::seconds(1));
		fps = frames;
		frames = 0;
	}
}

void LFEngine::Resize(GLuint width, GLuint height)
{
	LOGD("Engine: screen: width: %d  height: %d\n", width, height);
	gOBJRender->SetRenderRes(width, height);
	ui->SetResolution(width, height);
}

void LFEngine::SetDefaultFBO(GLuint fbo)
{
	default_fbo = fbo;
	gOBJRender->SetDefaultFBO(fbo);
}

void LFEngine::SetUI(int type, double sx, double sy)
{
	switch (type) {
	case 0:
		ui->FingerMove(sx, sy, gRenderCamera);
		break;
	case 1:
		ui->FingerDown(true, sx, sy);
		gOBJRender->SetFlag(true);
		break;
	case 2:
		ui->FingerDown(false, sx, sy);
		gOBJRender->SetFlag(false);
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

	RenderCamView &ref = gLFLoader->GetLightFieldAttrib().ref_cameras[id];
	this->gRenderCamera.setParameter(ref.GetPosition(),	ref.GetLookAt(), ref.GetUp(), 
		ref.GetFOVW(), ref.GetFOVH(), ref.GetCx(), ref.GetCy(), 
		ref.GetWidth(),	ref.GetHeight());
}

void LFEngine::SetZoomScale(float zoom_scale)
{
	if (zoom_scale <= 0.0f) {
		return;
	}

	glm::vec3 new_position = gRenderCamera.GetPosition() + (zoom_scale-1.0f) * gRenderCamera.GetLookAt();
	gRenderCamera.setParameter(new_position, gRenderCamera.GetLookAt(), gRenderCamera.GetUp());
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