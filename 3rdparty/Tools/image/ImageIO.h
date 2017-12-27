#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <string>
#include "image/Image.h"

class ImageIO
{
public:
	static Image Read(const std::string &filename, const unsigned int w, const unsigned int h);
};

#endif