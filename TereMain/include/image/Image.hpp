#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <cstdint>

struct Image
{
	std::shared_ptr<uint8_t> data;
	int width;
	int height;
	int chn;

	Image();
	Image(uint8_t *data, const int w, const int h, const int chn)
		: data(data), width(w), height(h), chn(chn) {}
	Image(std::shared_ptr<uint8_t> data, const int w, const int h, const int chn)
		: data(data), width(w), height(h), chn(chn) {}
};

#endif