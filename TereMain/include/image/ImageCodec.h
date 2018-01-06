#include <stdexcept>
#include <string>
#include "JpegCodec.h"
#include "WebpCodec.h"

using namespace std;

inline bool DecodeImageHeader(const std::string &filename, unsigned int &width, unsigned int &height)
{
	if (filename.length() < 5) {
		string errormsg = string("!![Error] DecodeHeader: file name too short: ") + filename;
		throw invalid_argument(errormsg);
	}

	std::string ext3 = filename.substr(filename.length() - 3, 3);
	std::string ext4 = filename.substr(filename.length() - 4, 4);

	if (ext3 == "jpg" || ext4 == "jpeg" || ext3 == "JPG" || ext4 == "JPEG") {
		return DecodeJpegHeader(filename, width, height);
	}
	else if (ext4 == "webp" || ext4 == "WEBP") {
		//return DecodeWebpHeader(filename, width, height);
		return false;
	}
	else {
		string errormsg = string("!![Error] DecodeImage: Unsupported image extension: ") + ext4;
		throw runtime_error(errormsg);
	}
}

inline bool DecodeImage(const std::string &filename, unsigned char *buf, const unsigned int bufsize,
	const unsigned int width, const unsigned int height)
{
	if (filename.length() < 5) {
		string errormsg = string("!![Error] DecodeImage: file name too short: ") + filename;
		throw invalid_argument(errormsg);
	}

	std::string ext3 = filename.substr(filename.length() - 3, 3);
	std::string ext4 = filename.substr(filename.length() - 4, 4);

	if (ext3 == "jpg" || ext4 == "jpeg" || ext3 == "JPG" || ext4 == "JPEG") {
		return DecodeJpeg(filename, buf, bufsize, width, height);
	}
	else if (ext4 == "webp" || ext4 == "WEBP") {
		return DecodeWebp(filename, buf, bufsize, width, height);
	}
	else {
		string errormsg = string("!![Error] DecodeImage: Unsupported image extension: ") + ext4;
		throw runtime_error(errormsg);
	}
}
