//
// Created by YuHuangjie on 6/14/2017.
//

#include <webp/decode.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

/**
* Decompress a webp file.
*
* @param filename	The file name
* @param buf[out]	Decoded byte array. Pixels are stored from bottom to top and
*					from left to right (to be compatiable with OpenGL). Each
*					pixel if stored in RGB format, where each component takes
*					8 bits.
*					The caller must allocate the memory.
* @param bufsize	Size of buf (in bytes)
* @param weight	Desired width of decoded image
* @param height	Desired height of decoded image
*
* @return true if decompression succed, false otherwise
*/
inline bool DecodeWebp(const string &filename, unsigned char *buf, const unsigned int bufsize,
	const unsigned int width, const unsigned int height)
{
    if (buf == nullptr) {
        return false;
    }
	if (bufsize < width * height * 4) {
		string errormsg = string("!![Error] DecodeWebp: too small buf size: ") + TO_STRING(bufsize);
		throw invalid_argument(errormsg);
	}

    // open image file
    ifstream in_file(filename, fstream::binary);
    unsigned int webp_size;
    unsigned char *webp_buf = nullptr;

    if (!in_file.is_open()) {
		string errormsg = string("!![Error] DecodeWebp: open image_file failed: ") + filename;
		throw invalid_argument(errormsg);
    }

    // read raw image data into memory
    in_file.seekg(0, fstream::end);
    webp_size = (unsigned int)in_file.tellg();
    in_file.seekg(0, fstream::beg);
    webp_buf = new unsigned char[webp_size];
    in_file.read(reinterpret_cast<char*>(webp_buf), webp_size);
	in_file.close();

    // decode
    WebPDecoderConfig config;
    if (WebPInitDecoderConfig(&config) != 1) {
        throw runtime_error("init decode config failed");
    }

    config.options.no_fancy_upsampling = 1;
    config.options.use_scaling = true;
    config.options.scaled_width = width;
    config.options.scaled_height = height;
    config.options.flip = true;

    config.output.colorspace = MODE_RGBA;
    config.output.u.RGBA.rgba = buf;
    config.output.u.RGBA.stride = width * 4;
    config.output.u.RGBA.size = width * height * 4;
    config.output.is_external_memory = true;

    if (WebPDecode(webp_buf, webp_size, &config) != VP8_STATUS_OK) {
        delete[] webp_buf;
        webp_buf = nullptr;
        throw runtime_error("!![Error] DecodeWebp: WebPDecode failed: unknown reason");
    }
    delete[] webp_buf;
    webp_buf = nullptr;
	WebPFreeDecBuffer(&config.output);

    return true;
}

/**
* Retrieve information about a webp image without decompressing it.
*
* @param filename	The file name
* @param width		A reference to a int variable that will recieve the width (in Pixels)
*					of the webp image
* @param height		A reference to a int variable that will recieve the height (in Pixels)
*					of the webp image
*
* @return true if decompression succeed, false otherwise
*/
inline bool DecodeWebpHeader(const string &filename, unsigned int &width, unsigned int &height)
{
	ifstream in_file(filename, fstream::binary);
	unsigned int webp_size;
	unsigned char *webp_buf = nullptr;

	if (!in_file.is_open()) {
		string errormsg = string("!![Error] DecodeWebpHeader: open image_file failed: ") + filename;
		throw invalid_argument(errormsg);
	}

	in_file.seekg(0, fstream::end);
	webp_size = (unsigned int)in_file.tellg();
	in_file.seekg(0, fstream::beg);
	webp_buf = new unsigned char[webp_size];
	in_file.read(reinterpret_cast<char*>(webp_buf), webp_size);
	in_file.close();

	int _width = 0;
	int _height = 0;

	if (WebPGetInfo(webp_buf, webp_size, &_width, &_height) != VP8_STATUS_OK) {
		delete[] webp_buf;
		webp_buf = nullptr;
		throw runtime_error("!![Error] DecodeWebp: WebPDecode failed: unknown reason");
	}
	width = _width;
	height = _height;
	delete[] webp_buf;
	webp_buf = nullptr;

	return true;
}