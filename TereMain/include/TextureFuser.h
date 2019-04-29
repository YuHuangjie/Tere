#ifndef TEXTUREFUSER_H
#define TEXTUREFUSER_H

#include <vector>
#include <memory>

#include "image/Image.hpp"

using std::vector;

class TextureFuser
{
public:
	// Construct by giving background texture
	TextureFuser(const size_t fbw, const size_t fbh, const Image &);

	// construct by given monochromatic background color
	TextureFuser(const size_t fbw, const size_t fbh, 
		const float bgR, const float bgG, const float bgB);

	TextureFuser(const TextureFuser &) = delete;
	TextureFuser& operator=(const TextureFuser &) = delete;
	~TextureFuser();

	// set foreground texture
	bool SetForeground(const unsigned int fgTex);

	// set background color
	bool SetBackground(const float r, const float g, const float b);
	bool SetBackground(const Image &);

	// render method
	int Render(const vector<int> &viewport) const;

private:
	bool IsConsistent() const;
	void Init();

	// background texture
	bool _monochromatic;
	unsigned int _bgTexture;
	float _bgR, _bgG, _bgB;

	// foreground texture
	unsigned int _fgTexture;

	// final rendering frame buffer
	size_t _fbw;
	size_t _fbh;
	unsigned int _fbo;
	unsigned int _fbTex;
	unsigned int _rbo;

	// shader program
	unsigned int _program;

	// buffer resources
	unsigned int _vertexArray;
	unsigned int _vertexBuffer;
	unsigned int _elementBuffer;

	// attrib/uniform location
	int _bgRLocation;
	int _bgGLocation;
	int _bgBLocation;
	int _monochromaticLocation;
	int _fgTextureLocation;
	int _bgTextureLocation;
};


#endif