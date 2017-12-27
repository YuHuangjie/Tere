#include <cstdint>
#include "ImageIO.h"

extern bool DecodeImage(const std::string &, unsigned char *, const unsigned int,
	const unsigned int, const unsigned int
);
extern bool DecodeImageHeader(const std::string &filename,
	unsigned int &width, unsigned int &height
);

/**
 * Decode image file into RGB format
 *
 * @param filename	The file name
 * @param w	Desired output image width. If w is set to 0, the width (in Pixels) of 
 *			the JPEG to be decompressed is used
 * @param h	Desired output image height. If h is set to 0, the height (in Pixels) of 
 *			the JPEG to be decompressed is used
 *
 * @return Decoded image
 */
Image ImageIO::Read(const std::string &filename, const unsigned int w, const unsigned int h)
{
	// Determine width and height (in Pixels) 
	unsigned int width = 0, height = 0;
	if (w == 0 || h == 0) {
		bool status = DecodeImageHeader(filename, width, height);
		if (!status) {
			return Image();
		}
	}
	else {
		width = w;
		height = h;
	}

	const uint32_t bufsize = width * height * 3;
	uint8_t *buf = new uint8_t[bufsize];

	bool status = DecodeImage(filename, buf, bufsize, width, height);

	if (!status) {
		return Image();
	}
	else {
		Image im(buf, bufsize, width, height);
		delete[] buf;
		buf = nullptr;
		return im;
	}
}