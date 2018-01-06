#ifndef IMAGE_H
#define IMAGE_H

#include <memory>

class Image
{
public:
	Image();
	Image(unsigned char *data, int size, unsigned int width, unsigned int height);

	unsigned char* GetData() const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	bool IsEmpty() const;

protected:
	std::unique_ptr<unsigned char> mpData;
	unsigned int mWidth;
	unsigned int mHeight;
};

#include "Image.inl"

#endif