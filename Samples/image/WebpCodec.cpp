#include <fstream>
#include <stdexcept>
#include <string>

#include <webp/decode.h>

using namespace std;

/**
* Decompress a webp file.
*
* @param filename	The file name
* @param buf[out]	Decoded byte array. Pixels are stored from top to bottom and
*					from left to right (to be compatiable with OpenGL). Each
*					pixel if stored in BGR format, where each component takes
*					8 bits.
*					The caller must allocate the memory.
* @param bufsize	Size of buf (in bytes)
* @param weight		Desired width of decoded image
* @param height		Desired height of decoded image
*
* @return true if decompression succed, exception otherwise
*/
bool WebpDecoder(const char *filename, const int width, const int height,
	void *buf, const size_t bufsize)
{
    if (buf == nullptr) {
        return false;
    }
	if (bufsize < width * height * 3) {
		string errormsg = string("[Error] DecodeWebp: too small buf size");
		throw runtime_error(errormsg);
	}

    // open image file
    ifstream in_file(filename, fstream::binary);
    unsigned int webp_size;
    unsigned char *webp_buf = nullptr;

    if (!in_file.is_open()) {
		string errormsg = string("[Error] DecodeWebp: open image_file failed: ") + filename;
		throw runtime_error(errormsg);
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
        throw runtime_error("[Error] DecodeWebp: init decode config failed");
    }

    config.options.no_fancy_upsampling = 1;
    config.options.use_scaling = true;
    config.options.scaled_width = width;
    config.options.scaled_height = height;
    config.options.flip = false;

    config.output.colorspace = MODE_BGRA;
    config.output.u.RGBA.rgba = buf;
    config.output.u.RGBA.stride = width * 4;
    config.output.u.RGBA.size = width * height * 4;
    config.output.is_external_memory = true;

    if (WebPDecode(webp_buf, webp_size, &config) != VP8_STATUS_OK) {
        delete[] webp_buf;
        webp_buf = nullptr;
        throw runtime_error("[Error] DecodeWebp: WebPDecode failed: unknown reason");
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
* @param width		A pointer to a int variable that will recieve the width (in Pixels)
*					of the webp image
* @param height		A pointer to a int variable that will recieve the height (in Pixels)
*					of the webp image
*
* @return true if decompression succeed, exception otherwise
*/
bool WebpHeaderDecoder(const char *filename, int *width, int *height)
{
	ifstream in_file(filename, fstream::binary);
	unsigned int webp_size;
	unsigned char *webp_buf = nullptr;

	if (!in_file.is_open()) {
		string errormsg = string("[Error] WebpHeaderDecoder: open image_file failed: ") + filename;
		throw runtime_error(errormsg);
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
		throw runtime_error("[Error] WebpHeaderDecoder: WebPDecode failed: unknown reason");
	}
	*width = _width;
	*height = _height;
	delete[] webp_buf;
	webp_buf = nullptr;

	return true;
}