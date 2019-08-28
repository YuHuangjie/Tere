#include "RenderUtils.h"
#include "Error.h"
#include "Platform.h"
#include "common/Shader.hpp"

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	// the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if defined GL_WIN ||defined GL_OSX
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 
		static_cast<int>(width), static_cast<int>(height));
#elif defined GL_IOS || defined GL_ANDROID
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 
		static_cast<int>(width), static_cast<int>(height));
#endif
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// check if the framebuffer is actually complete
	GLenum code = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (code != GL_FRAMEBUFFER_COMPLETE) {
		RETURN_ON_ERROR("Framebuffer is not complete! Error code: %d", code);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

unsigned int GetTextureFromImage(const uint8_t * const bg,
	const int bgW, const int bgH, const int channels)
{
	unsigned int tex = 0;

	if (bgW == 0 || bgH == 0 || !bg) {
		RETURN_ON_ERROR("bgW == 0 || bgH == 0 || !bg");
	}
	if (channels != 3 && channels != 4) {
		RETURN_ON_ERROR("channels != 3 && channels != 4");
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, bgW, bgH);
	if (channels == 4) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bgW, bgH, GL_RGBA, GL_UNSIGNED_BYTE, bg);
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bgW, bgH, GL_RGB, GL_UNSIGNED_BYTE, bg);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

void DestroyTexture(unsigned int &tex)
{
	if (tex <= 0) {
		return;
	}
	glDeleteTextures(1, &tex);
	tex = 0;
}

void EnableMultiSample(bool flag)
{
#ifdef GL_MULTISAMPLE
	if (flag) { glEnable(GL_MULTISAMPLE); }
	else      { glDisable(GL_MULTISAMPLE); }
#elif defined GL_MULTISAMPLE_EXT
	if (flag) { glEnable(GL_MULTISAMPLE_EXT); }
	else      { glDisable(GL_MULTISAMPLE_EXT); }
#else
#error("GL_MULTISAMPLE not found");
#endif
}