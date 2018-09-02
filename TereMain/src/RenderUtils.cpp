#include "RenderUtils.h"
#include "common/Shader.hpp"
#include "common/Log.hpp"

unsigned int LoadShaders(const char * vertex_code, const char * fragment_code)
{
	// Compile shaders
	Shader shader(vertex_code, fragment_code);
	return shader.ID;
}

bool GenFrameBuffer(GLuint &fbo, GLuint &tex, GLuint &rbo,
	const size_t width, const size_t height)
{
	// Create frame buffer for offline rendering
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &tex);
	glGenRenderbuffers(1, &rbo);

	// bind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// bind color texture
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(width), 
		static_cast<int>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	// the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if GL_WIN || GL_OSX
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 
		static_cast<int>(width), static_cast<int>(height));
#elif GL_ES3_IOS || GL_ES3_ANDROID
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 
		static_cast<int>(width), static_cast<int>(height));
#endif
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// check if the framebuffer is actually complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOGE("Framebuffer is not complete!\n");
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}