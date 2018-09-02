#ifndef TEXTUREFUSER_H
#define TEXTUREFUSER_H

#include <vector>

using std::vector;

class Image;

class TextureFuser
{
public:
	// Construct by giving background texture
	//explicit TextureFuser(const size_t fbw, const size_t fbh, const Image& bg);

	// construct by given monochromatic background color
	TextureFuser(const size_t fbw, const size_t fbh, 
		const float bgR, const float bgG, const float bgB);

	TextureFuser(const TextureFuser &) = delete;
	TextureFuser& operator=(const TextureFuser &) = delete;
	~TextureFuser();

	// set foreground texture
	bool SetForeground(unsigned int fgTex);

	// set background color
	bool SetBackground(float r, float g, float b);
	//bool SetBackground(const Image &);

	// render method
	int Render(const vector<int> &viewport) const;

private:
	bool IsConsistent() const;

	// background texture
	bool _monochromatic;
	//unsigned int _bgTexture;
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
	int _fgTextureLocation;
	//unsigned int _bgTextureLocation;
};


#endif