#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "LFEngine.h"			// Tere API

#include "ProfileIO.hpp"		// Parse profile.txt
#include "mesh/Geometry.hpp"	// Read mesh
#include "image/ImageCodec.h"	// Image decoder
#include "Assert.h"				// ASSERT macro

namespace {
	const int WINDOW_WIDTH = 1024;
	const int WINDOW_HEIGHT = 1024;

	// light field engine  
	LFEngine *myEngine = nullptr;

	// main window
	double mx, my;
	GLFWwindow *gWindow;
	float zoom_scale = 1.0f;

	void Usage(void)
	{
		std::cout << "Usage:    " << "TereSample <profile>" << std::endl;
	}

	void GLAPIENTRY GLErrorCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length,
		const GLchar* message, const void* userParam)
	{
		if (type == GL_DEBUG_TYPE_ERROR) {
			std::fprintf(stderr, "GL ERROR: severity = 0x%x, message = %s\n",
				severity, message);
		}
	}

	void InitGLContext(int width, int height)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#if PLATFORM_OSX
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 1);
		gWindow = glfwCreateWindow(width, height, "TereSample", nullptr, nullptr);
		glfwIconifyWindow(gWindow);
		glfwMakeContextCurrent(gWindow);

		glewExperimental = true;
		glewInit();

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GLErrorCallback, 0);
	}

	void RenderKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
	}

	void RenderMouseCallback(GLFWwindow *window, int button, int action, int mod)
	{
		// Drag left button to interact with scene
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			glfwGetCursorPos(window, &mx, &my);
			myEngine->SetUI(UIType::TOUCH, mx, my);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			glfwGetCursorPos(window, &mx, &my);
			myEngine->SetUI(UIType::LEAVE, mx, my);
		}
	}

	void RenderCursorCallback(GLFWwindow *window, double xpos, double ypos)
	{
		myEngine->SetUI(UIType::MOVE, xpos, ypos);
	}

	void RenderScrollCallback(GLFWwindow *window, double xpos, double ypos)
	{
		static float zoomScale = 1.0f;
		zoom_scale += ypos * 0.1f;
		zoom_scale = myEngine->SetZoomScale(zoom_scale);
	}

	void ResizeCallback(GLFWwindow *window, int width, int height)
	{
		if (width != 0 && height != 0) {
			myEngine->Resize(width, height);
		}
	}

	void SetGLCallbacks(void)
	{
		glfwSetKeyCallback(gWindow, RenderKeyCallback);
		glfwSetMouseButtonCallback(gWindow, RenderMouseCallback);
		glfwSetCursorPosCallback(gWindow, RenderCursorCallback);
		glfwSetScrollCallback(gWindow, RenderScrollCallback);
		glfwSetWindowSizeCallback(gWindow, ResizeCallback);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		Usage();
		return -1;
	}

	try {
		string profileFile(argv[1]);
		Profile profile = ReadProfile(profileFile);
		profile.mode = RENDER_MODE::LINEAR;

		// initialize OpenGL
		InitGLContext(WINDOW_WIDTH, WINDOW_HEIGHT);

		// set window callbacks
		SetGLCallbacks();

		/* set up render engine */
		myEngine = new LFEngine(profile.nCams, profile.mode);

		// set cameras
		for (size_t i = 0; i < profile.nCams; ++i) {
			ASSERT(myEngine->SetCamera(i, profile.intrins[i], profile.extrins[i], false, true));
		}

		// set geometry
		Geometry geo = Geometry::FromFile(profile.mesh);
		ASSERT(geo.HasVertex());

		if (geo.HasFace()) {
			ASSERT(myEngine->SetGeometry(geo.vertices.data(), geo.vertices.size() * sizeof(float),
				geo.indices.data(), geo.indices.size() * sizeof(int32_t), false));
		}
		else {
			ASSERT(myEngine->SetGeometry(geo.vertices.data(), geo.vertices.size() * sizeof(float), false));
		}

		// set image data
		myEngine->RegisterDecFunc(JpegHeaderDecoder, JpegDecoder);
#pragma omp parallel for
		for (int i = 0; i < profile.nCams; ++i) {
			ASSERT(myEngine->SetRefImage(i, profile.imageList[i], 1.f));
		}

		// set scene settings
		switch (profile.mode)
		{
		case RENDER_MODE::LINEAR:
			myEngine->SetRows(profile.rows); break;
		case RENDER_MODE::SPHERE:
			break;
		case RENDER_MODE::ALL:
			break;
		default: break;
		}

		// Must call configure after data are uploaded
		ASSERT(myEngine->HaveSetScene());

		//myEngine->SetLocationOfReferenceCamera(0);

		// set screen viewport
		myEngine->Resize(WINDOW_WIDTH, WINDOW_HEIGHT);

		// start background fps counting thread
		myEngine->StartFPSThread();

		// set background image
		//ASSERT(myEngine->SetBackground("view.jpg"));
		myEngine->SetBackground(0.3f, 0.f, 0.f);

		glfwRestoreWindow(gWindow);

		// main play loop
		while (!glfwWindowShouldClose(gWindow)) {
			// Draw scene
			glfwMakeContextCurrent(gWindow);

			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			myEngine->Draw();

			glfwSwapBuffers(gWindow);
			glfwPollEvents();
		}

		// release resource
		delete myEngine;
		myEngine = nullptr;
	}
	catch (std::exception &e) {
		std::cerr << "runtime error occured: " << e.what() << std::endl;
		system("pause");
	}
}