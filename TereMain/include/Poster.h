#ifndef POSTER_H
#define POSTER_H

#include <vector>

using std::vector;

class Poster
{
public:
	Poster();
	Poster(unsigned int texture);
	~Poster();
	Poster(const Poster&) = delete;
	Poster& operator=(const Poster &) = delete;

	// set texture to render
	bool SetTexture(unsigned int texture);

	// render method
	int Render(const vector<int> &viewport) const;

private:
	bool IsConsistent() const;
	void Init();

	// the texture to be rendered
	unsigned int _texture;

	// shader program
	unsigned int _program;

	// buffer resources
	unsigned int _vertexArray;
	unsigned int _vertexBuffer;
	unsigned int _elementBuffer;

	// texture location
	int _imageLocation;
};

#endif