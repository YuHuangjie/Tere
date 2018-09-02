#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <string>
#include "Image.hpp"

class ImageIO
{
public:
	static Image Read(const std::string &filename, 
		const unsigned int w = 0, const unsigned int h = 0);
};

#include "ImageIO.inl"

#endif