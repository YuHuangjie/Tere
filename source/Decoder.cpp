#include "Decoder.h"
#include <stdexcept>

bool Decompress(std::string filename, unsigned char *buf, int width, int height)
{
	if (filename.length() < 5) {
		throw std::runtime_error("invalid image path");
	}

	std::string ext3 = filename.substr(filename.length() - 3, 3);
	std::string ext4 = filename.substr(filename.length() - 4, 4);

	try {
		if (ext3 == "jpg" || ext4 == "jpeg" || ext3 == "JPG" || ext4 == "JPEG") {
			return DecompressJpeg(filename, buf, width, height);
		}
		else if (ext4 == "webp" || ext4 == "WEBP") {
			return DecompressWebp(filename, buf, width, height);
		}
		else {
			throw std::runtime_error("Unsupported image extension");
		}
	}
	catch (std::runtime_error &e) {
		throw e;
	}
}
