#include <string>
#include <stdexcept>
#include "Image.h"

Image::Image()
	: mWidth(0), mHeight(0)
{}

Image::Image(unsigned char *data, int size, unsigned int width, unsigned int height)
	: mWidth(width), mHeight(height)
{
	if (size != width * height * 3) {
		throw std::invalid_argument(std::string("!![Error] VideoSplitter: Invalid size ") +
			TO_STRING(size));
	}

	unsigned char *_data = new unsigned char[size]();
	std::memcpy(_data, data, size);
	mpData.reset(_data);
}

unsigned int Image::GetWidth() const
{
	return mWidth;
}

unsigned int Image::GetHeight() const
{
	return mHeight;
}

unsigned char* Image::GetData() const
{
	return mpData.get();
}

bool Image::IsEmpty() const
{
	return (mpData ? false : true);
}