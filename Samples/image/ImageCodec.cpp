#include <stdexcept>
#include <string>
#include "ImageCodec.h"

using namespace std;

bool ImageHeaderDecoder(const char *_filename, int *width, int *height)
{
	std::string filename(_filename);

	if (filename.length() < 5) {
		string errormsg = string("[Error] DecodeHeader: file name too short: ") + filename;
		throw runtime_error(errormsg);
	}

	std::string ext3 = filename.substr(filename.length() - 3, 3);
	std::string ext4 = filename.substr(filename.length() - 4, 4);

	if (ext3 == "jpg" || ext4 == "jpeg" || ext3 == "JPG" || ext4 == "JPEG") {
		return JpegHeaderDecoder(_filename, width, height);
	}
	/*else if (ext4 == "webp" || ext4 == "WEBP") {
		return WebpHeaderDecoder(_filename, width, height);
	}*/
	else {
		string errormsg = string("[Error] DecodeImage: Unsupported image format: ") + ext4;
		throw runtime_error(errormsg);
	}
}

bool ImageDecoder(const char *_filename, const int width, const int height,
	void *buf, const size_t bufsize)
{
	std::string filename(_filename);

	if (filename.length() < 5) {
		string errormsg = string("[Error] DecodeImage: file name too short: ") + filename;
		throw runtime_error(errormsg);
	}

	std::string ext3 = filename.substr(filename.length() - 3, 3);
	std::string ext4 = filename.substr(filename.length() - 4, 4);

	if (ext3 == "jpg" || ext4 == "jpeg" || ext3 == "JPG" || ext4 == "JPEG") {
		return JpegDecoder(_filename, width, height, buf, bufsize);
	}
	/*else if (ext4 == "webp" || ext4 == "WEBP") {
		return WebpDecoder(_filename, width, height, buf, bufsize);
	}*/
	else {
		string errormsg = string("[Error] DecodeImage: Unsupported image format: ") + ext4;
		throw runtime_error(errormsg);
	}
}
