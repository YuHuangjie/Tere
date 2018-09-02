#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "LFEngine.h"
#include "common/Shader.hpp"
#include "UIType.h"

namespace {
	const int WINDOW_WIDTH = 360;
	const int WINDOW_HEIGHT = 640;

	// light field engine  
	LFEngine *myEngine = nullptr;

	// main window
	double mx, my;
	GLFWwindow *gWindow;
	float zoom_scale = 1.0f;

	// screen shot window
	GLFWwindow *screen_shot_window = nullptr;
	unsigned char *screen_shot_buffer = nullptr;
	bool need_screen_shot = false;
	double screen_shot_x = 0, screen_shot_y = 0;
	double screen_shot_width = 0, screen_shot_height = 0;
	unsigned int screen_shot_VAO = 0, screen_shot_VBO = 0, screen_shot_EBO = 0;
	unsigned int screen_shot_texture = 0;
	Shader *screen_shot_shader = nullptr;

	// Instrunction on using this program
	void Usage(void);

	// OpenGL context and callbacks setup
	void InitGLContext(int window_width, int window_height);
	void SetGLCallbacks(void);
	void RenderKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
	void RenderMouseCallback(GLFWwindow *window, int button, int action, int mod);
	void RenderCursorCallback(GLFWwindow *window, double xpos, double ypos);
	void RenderScrollCallback(GLFWwindow *window, double xpos, double ypos);
	void ResizeCallback(GLFWwindow *window, int width, int height);
	void GLAPIENTRY GLErrorCallback(GLenum source, GLenum type, 
		GLuint id, GLenum severity, GLsizei length, 
		const GLchar* message, const void* userParam);

	// Visualize screen shot
	void VisualizeScreenShot(unsigned char *buffer, int width, int height);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		Usage();
		return -1;
	}

	try {
		string profilePathName(argv[1]);

		// initialize OpenGL
		InitGLContext(WINDOW_WIDTH, WINDOW_HEIGHT);

		// set window callbacks
		SetGLCallbacks();

		// set up render engine
		myEngine = new LFEngine(profilePathName);
		//myEngine->SetLocationOfReferenceCamera(12);
		myEngine->Resize(WINDOW_WIDTH, WINDOW_HEIGHT);
		myEngine->StartFPSThread();
		//myEngine->SetBackground("view.jpg");
		myEngine->SetBackground(0.2f, 0.3f, 0.4f);
		glfwRestoreWindow(gWindow);

		// main play loop
		while (!glfwWindowShouldClose(gWindow)) {
			// Draw scene
			glfwMakeContextCurrent(gWindow);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
			myEngine->Draw();
			glfwSwapBuffers(gWindow);

			// Draw screen shot
			if (need_screen_shot) {
				if (screen_shot_buffer == nullptr) {
					screen_shot_buffer = new unsigned char[
						(int)screen_shot_height*(int)screen_shot_width * 4
					];
				}
				myEngine->GetScreenShot(screen_shot_buffer, (int)screen_shot_x, (int)screen_shot_y,
					(int)screen_shot_width, (int)screen_shot_height);
				VisualizeScreenShot(screen_shot_buffer, (int)screen_shot_width, (int)screen_shot_height);
			}
			if (!need_screen_shot && screen_shot_buffer) {
				delete[] screen_shot_buffer;
				screen_shot_buffer = nullptr;
			}

			// glfw poll events
			glfwPollEvents();

		}

		// release resource
		delete myEngine;
		myEngine = nullptr;
	}
	catch (std::exception &e) {
		cerr << "runtime error occured: " << e.what() << endl;
		system("pause");
	}
}

namespace {
	void Usage(void)
	{
		cout << "Usage:    " << "TereSample <profile>" << endl;
	}

	void InitGLContext(int width, int height)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#if PLATFORM_OSX
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 1);
		gWindow = glfwCreateWindow(width, height, "TereSample", nullptr, nullptr);
		glfwIconifyWindow(gWindow);
		glfwMakeContextCurrent(gWindow);

		glewExperimental = true;        // Needed in core profile
		GLenum err = glewInit();
		std::string error;
		if (err != GLEW_OK) {
			error = (const char *)(glewGetErrorString(err));
			throw runtime_error(error);
		}

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GLErrorCallback, 0);
	}

	void GLAPIENTRY GLErrorCallback(GLenum source, GLenum type, GLuint id, 
		GLenum severity, GLsizei length, 
		const GLchar* message, const void* userParam)
	{
		std::fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}

	void SetGLCallbacks(void)
	{
		glfwSetKeyCallback(gWindow, RenderKeyCallback);
		glfwSetMouseButtonCallback(gWindow, RenderMouseCallback);
		glfwSetCursorPosCallback(gWindow, RenderCursorCallback);
		glfwSetScrollCallback(gWindow, RenderScrollCallback);
		glfwSetWindowSizeCallback(gWindow, ResizeCallback);
	}

	void RenderKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			// handle key events
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
		// Drag right button to take screen shot
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			glfwGetCursorPos(window, &screen_shot_x, &screen_shot_y);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			if (x < screen_shot_x) { std::swap(x, screen_shot_x); }
			if (y < screen_shot_y) { std::swap(y, screen_shot_y); }
			screen_shot_width = x - screen_shot_x;
			screen_shot_height = y - screen_shot_y;

			if (screen_shot_width == 0 || screen_shot_height == 0) {
				return;
			}

			screen_shot_y = WINDOW_HEIGHT - screen_shot_y - screen_shot_height;
			need_screen_shot = true;
		}
	}

	void RenderCursorCallback(GLFWwindow *window, double xpos, double ypos)
	{
		myEngine->SetUI(UIType::MOVE, xpos, ypos);
	}

	void RenderScrollCallback(GLFWwindow *window, double xpos, double ypos)
	{
		float scale_factor = 0.1f;
		zoom_scale = 1 + (float)ypos * scale_factor;
		myEngine->SetZoomScale(zoom_scale);
	}

	void ResizeCallback(GLFWwindow *window, int width, int height)
	{
		if (width != 0 && height != 0) {
			myEngine->Resize(width, height);
		}
	}

	void VisualizeScreenShot(unsigned char *buffer, int width, int height)
	{
		if (!need_screen_shot) { return; }
		if (screen_shot_window == nullptr) {
			screen_shot_window = glfwCreateWindow(width*3, height*3, "ScreenShot", nullptr, nullptr);
		}
		glfwMakeContextCurrent(screen_shot_window);
		if (glfwWindowShouldClose(screen_shot_window)) {
			glfwDestroyWindow(screen_shot_window);
			screen_shot_window = nullptr;
			need_screen_shot = false;
			glDeleteVertexArrays(1, &screen_shot_VAO);
			glDeleteBuffers(1, &screen_shot_VBO);
			glDeleteBuffers(1, &screen_shot_EBO);
			screen_shot_VAO = 0;
			screen_shot_VBO = 0;
			screen_shot_EBO = 0;
			delete screen_shot_shader;
			screen_shot_shader = nullptr;
			return;
		}
		if (buffer == nullptr || width <= 0 || height <= 0) { return; }

		// preparation
		if (screen_shot_VAO == 0) {
			// shaders
			const string vertex_code = \
				"#version 330 core							\n"
				"											\n"
				"layout(location = 0) in vec3 pos;			\n"
				"layout(location = 1) in vec2 tex_coord;	\n"
				"											\n"
				"out vec2 my_tex_coord;						\n"
				"											\n"
				"void main()								\n"
				"{											\n"
				"	gl_Position = vec4(pos, 1.0);			\n"
				"	my_tex_coord = tex_coord;				\n"
				"}											\n";
			const string frag_code = \
				"#version 330 core							\n"
				"out vec4 color;							\n"
				"											\n"
				"in vec2 my_tex_coord;						\n"
				"											\n"
				"uniform sampler2D my_texture;				\n"
				"											\n"
				"void main()								\n"
				"{											\n"
				"	color = texture(my_texture, my_tex_coord);	\n"
				"}											\n";
			
			if (screen_shot_shader == nullptr) {
				screen_shot_shader = new Shader(vertex_code, frag_code);
			}

			float vertices[] = {
				// positions			// texture coords
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f
			};
			unsigned int indices[] = {
				0, 1, 3,
				1, 2, 3
			};
			glGenVertexArrays(1, &screen_shot_VAO);
			glGenBuffers(1, &screen_shot_VBO);
			glGenBuffers(1, &screen_shot_EBO);

			glBindVertexArray(screen_shot_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, screen_shot_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screen_shot_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);

			glGenTextures(1, &screen_shot_texture);
			glBindTexture(GL_TEXTURE_2D, screen_shot_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// upload texture
		glBindTexture(GL_TEXTURE_2D, screen_shot_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

		// draw
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(screen_shot_shader->ID);
		glBindVertexArray(screen_shot_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(screen_shot_window);
	}

}
