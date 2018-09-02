#ifndef RENDERUTILS_H
#define RENDERUTILS_H

// compile vertex, fragment shaders
unsigned int LoadShaders(const char * vs_code, const char * frag_code);

// Create framebuffer for offline rendering
// Note: texture filter is set to GL_NEAREST
bool GenFrameBuffer(unsigned int &fbo, unsigned int &tex, unsigned int &rbo,
	const size_t width, const size_t height);

#endif