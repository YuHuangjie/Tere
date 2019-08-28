#include <stdexcept>

#include "Poster.h"
#include "GLHeader.h"
#include "RenderUtils.h"
#include "shader/poster_vs.h"
#include "shader/poster_frag.h"
#include "Error.h"

using std::runtime_error;

Poster::Poster()
	: _texture(0),
	_program(0),
	_vertexArray(0),
	_vertexBuffer(0),
	_elementBuffer(0),
    _fbo(0)
{
	Init();

	if (!IsConsistent()) {
		THROW_ON_ERROR("Poster is Inconsistent");
	}
}

Poster::Poster(unsigned int texture)
	: _texture(texture),
	_program(0),
	_vertexArray(0),
	_vertexBuffer(0),
	_elementBuffer(0),
    _fbo(0)
{
	Init();

	if (!IsConsistent()) {
		THROW_ON_ERROR("Poster is Inconsistent");
	}
}

Poster::~Poster()
{
	glDeleteProgram(_program);
	glDeleteVertexArrays(1, &_vertexArray);
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_elementBuffer);
}


bool Poster::Render(const vector<int>& viewport) const
{
	if (!IsConsistent()) {
		RETURN_ON_ERROR("Poster: Abort rendering due to inconsistency\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glUseProgram(_program);
	glDisable(GL_DEPTH_TEST);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glUniform1i(_imageLocation, 0);

	glBindVertexArray(_vertexArray);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	return true;
}

bool Poster::SetTexture(unsigned int texture)
{
	if (texture <= 0) {
		return false;
	}
	_texture = texture;
	return true;
}

bool Poster::IsConsistent() const
{
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
	if (_imageLocation < 0) {
		return false;
	}
	return true;
}

void Poster::Init()
{
	// compile shader
	if ((_program = LoadShaders(poster_vs_code, poster_frag_code)) <= 0) {
		throw runtime_error("Poster: compile shader failed");
	}

	// get uniform location
	_imageLocation = glGetUniformLocation(_program, "image");

	// generate vertex buffer
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

void Poster::SetScreenFBO(unsigned int fbo)
{
    _fbo = fbo;
}
