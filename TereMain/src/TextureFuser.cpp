#include <stdexcept>

#include "common/Common.hpp"
#if GL_WIN || GL_OSX
#include <GL/glew.h>
#elif GL_ES3_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif GL_ES3_ANDROID
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include "TextureFuser.h"
#include "RenderUtils.h"
#include "shader/fuser_vs.h"
#include "shader/fuser_frag.h"
#include "common/Log.hpp"

using std::runtime_error;

TextureFuser::TextureFuser(const size_t fbw, const size_t fbh, 
	const float bgR, const float bgG, const float bgB)
	: _monochromatic(true),
	_bgTexture(0),
	_bgR(bgR),
	_bgG(bgG),
	_bgB(bgB),
	_fgTexture(0),
	_fbw(fbw),
	_fbh(fbh),
	_fbo(0),
	_fbTex(0),
	_rbo(0),
	_program(0),
	_vertexArray(0),
	_vertexBuffer(0),
	_elementBuffer(0),
	_bgRLocation(-1),
	_bgGLocation(-1),
	_bgBLocation(-1),
	_monochromaticLocation(-1),
	_fgTextureLocation(-1),
	_bgTextureLocation(-1)
{
	Init();

	// after initialization, check consistency
	if (!IsConsistent()) {
		throw runtime_error("TextureFuser is inconsistent");
	}
}

TextureFuser::TextureFuser(const size_t fbw, const size_t fbh, const Image& bg)
	: _monochromatic(false),
	_bgTexture(0),
	_bgR(0.f),
	_bgG(0.f),
	_bgB(0.f),
	_fgTexture(0),
	_fbw(fbw),
	_fbh(fbh),
	_fbo(0),
	_fbTex(0),
	_rbo(0),
	_program(0),
	_vertexArray(0),
	_vertexBuffer(0),
	_elementBuffer(0),
	_bgRLocation(-1),
	_bgGLocation(-1),
	_bgBLocation(-1),
	_monochromaticLocation(-1),
	_fgTextureLocation(-1),
	_bgTextureLocation(-1)
{
	Init();

	// set background texture
	if (!SetBackground(bg)) {
		throw runtime_error("TextureFuser set background image failed");
	}

	// after initialization, check consistency
	if (!IsConsistent()) {
		throw runtime_error("TextureFuser is inconsistent");
	}
}

TextureFuser::~TextureFuser()
{
	glDeleteFramebuffers(1, &_fbo);
	glDeleteTextures(1, &_fbTex);
	glDeleteBuffers(1, &_rbo);
	glDeleteProgram(_program);
	glDeleteVertexArrays(1, &_vertexArray);
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_elementBuffer);
	glDeleteTextures(1, &_fgTexture);
}

int TextureFuser::Render(const vector<int> &viewport) const
{
	if (!IsConsistent()) {
		LOGE("TextureFuser: Abort rendering due to inconsistency\n");
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glUseProgram(_program);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	// upload uniform variables
	if (_monochromatic) {
		glUniform1f(_bgRLocation, _bgR);
		glUniform1f(_bgGLocation, _bgG);
		glUniform1f(_bgBLocation, _bgB);
	}
	glUniform1i(_monochromaticLocation, static_cast<int>(_monochromatic));
	
	// bind foreground texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _fgTexture);
	glUniform1i(_fgTextureLocation, 0);

	// bind background texture
	if (!_monochromatic) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _bgTexture);
		glUniform1i(_bgTextureLocation, 1);
	}

	glBindVertexArray(_vertexArray);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _fbTex;
}

void TextureFuser::Init()
{
	// compile shader
	if ((_program = LoadShaders(fuser_vs_code, fuser_frag_code)) <= 0) {
		throw runtime_error("TextureFuser: compile shader failed");
	}

	// generate framebuffer
	if (!GenFrameBuffer(_fbo, _fbTex, _rbo, _fbw, _fbh)) {
		throw runtime_error("TextureFuser: generate frame buffer failed");
	}

	// get uniform variable locations
	_bgRLocation = glGetUniformLocation(_program, "bgR");
	_bgGLocation = glGetUniformLocation(_program, "bgG");
	_bgBLocation = glGetUniformLocation(_program, "bgB");
	_bgTextureLocation = glGetUniformLocation(_program, "bgTexture");
	_fgTextureLocation = glGetUniformLocation(_program, "fgTexture");
	_monochromaticLocation = glGetUniformLocation(_program, "monochromatic");

	// generate vertex array
	float vertices[] = {
		// positions  // texture coords
		1.0f,  1.0f,    1.0f, 1.0f, // top right
		1.0f, -1.0f,    1.0f, 0.0f, // bottom right
		-1.0f, -1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &_vertexArray);
	glGenBuffers(1, &_vertexBuffer);
	glGenBuffers(1, &_elementBuffer);

	glBindVertexArray(_vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//	position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//	color attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

bool TextureFuser::IsConsistent() const
{
	if (_fbo <= 0) {
		return false;
	}
	if (_fbTex <= 0) {
		return false;
	}
	if (_rbo <= 0) {
		return false;
	}
	if (_program <= 0) {
		return false;
	}
	if (_vertexArray <= 0) {
		return false;
	}
	if (_vertexBuffer <= 0) {
		return false;
	}
	if (_elementBuffer <= 0) {
		return false;
	}
	if (_fgTextureLocation <= 0) {
		return false;
	}

	if (_monochromatic && _bgRLocation < 0) {
		return false;
	}
	if (_monochromatic && _bgGLocation < 0) {
		return false;
	}
	if (_monochromatic && _bgBLocation < 0) {
		return false;
	}

	if (!_monochromatic && _bgTextureLocation < 0) {
		return false;
	}

	return true;
}

bool TextureFuser::SetForeground(unsigned int fgTex)
{
	if (fgTex <= 0) {
		return false;
	}

	_fgTexture = fgTex;
	return true;
}

bool TextureFuser::SetBackground(float r, float g, float b)
{
	_bgR = r;
	_bgG = g;
	_bgB = b;

	// delete background texture is any
	DestroyTexture(_bgTexture);

	_monochromatic = true;
	return true;
}

bool TextureFuser::SetBackground(const Image &image)
{
	unsigned int tex = GetTextureFromImage(image);

	if (tex == 0) {
		return false;
	}

	// delete background texture is any
	DestroyTexture(_bgTexture);

	_bgTexture = tex;
	_monochromatic = false;
	return true;
}