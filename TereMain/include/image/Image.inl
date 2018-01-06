#include <string>
#include <stdexcept>
#include "Image.hpp"

inline Image::Image()
	: mWidth(0), mHeight(0)
{}

inline Image::Image(unsigned char *data, int size, unsigned int width, unsigned int height)
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

inline unsigned int Image::GetWidth() const
{
	return mWidth;
}

inline unsigned int Image::GetHeight() const
{
	return mHeight;
}

inline unsigned char* Image::GetData() const
{
	return mpData.get();
}

inline bool Image::IsEmpty() const
{
	return (mpData ? false : true);
}