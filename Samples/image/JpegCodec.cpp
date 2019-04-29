#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>

#include <turbojpeg.h>

#include "ImageCodec.h"

using namespace std;

/**
 * Decompress a jpeg file.
 *
 * @param filename	The file name
 * @param buf[out]	Decoded byte array. Pixels are stored from top to bottom and 
 *					from left to right (to be compatiable with OpenGL). Each 
 *					pixel if stored in BGR format, where each component takes
 *					8 bits. 
 *					The caller must allocate the memory.
 * @param bufsize	Size of buf (measured in bytes)
 * @param width		Desired width of decoded image. If width is set to 0, the width of 
 *					the JPEG to be decoded will be used
 * @param height	Desired height of decoded image. If height is set to 0, the height of 
 *					the JPEG to be decoded will be used
 *
 * @return true if decompression succed, exception otherwise
 */
bool JpegDecoder(const char *filename, const int width, const int height,
	void *buf, const size_t bufsize)
{
    if (buf == nullptr) {
        return false;
    }
	if (bufsize < width * height * 3) {
		cerr << "[Error] JpegDecoder: too small buf size" << endl;
		return false;
	}

    ifstream in_file(filename, fstream::binary);
    unsigned int jpeg_size;
    unsigned char *jpeg_buf = nullptr;

    if (!in_file.is_open()) {
		cerr << "[Error] JpegDecoder: open image_file failed: " << endl;
		return false;
    }

    in_file.seekg(0, fstream::end);
    jpeg_size = (unsigned int)in_file.tellg();
    in_file.seekg(0, fstream::beg);
    jpeg_buf = new unsigned char[jpeg_size];
    in_file.read(reinterpret_cast<char*>(jpeg_buf), jpeg_size);
	in_file.close();

    tjhandle decompressor = tjInitDecompress();
    int result = tjDecompress2(decompressor, jpeg_buf, jpeg_size, 
		static_cast<unsigned char*>(buf), width, 0, height, TJPF_BGR, 0);
    tjDestroy(decompressor);
    delete[] jpeg_buf;

    if (result != 0) {
		cerr << "[Error] DecodeJpeg: decompress fail " << tjGetErrorStr() << endl;
		return false;
    }

    return true;
}

/**
* Retrieve information about a JPEG image without decompressing it.
*
* @param filename	The file name
* @param width		A pointer to a int variable that will recieve the width (in Pixels)
*					of the JPEG image
* @param height		A pointer to a int variable that will recieve the height (in Pixels)
*					of the JPEG image
*
* @return true if decompression succeed, exception otherwise
*/
bool JpegHeaderDecoder(const char *filename, int *width, int *height)
{
	ifstream in_file(filename, fstream::binary);
	unsigned int jpeg_size;
	unsigned char *jpeg_buf = nullptr;

	if (!in_file.is_open()) {
		cerr << "[Error] JpegDecoder: open image_file failed: " << endl;
		return false;
	}

	in_file.seekg(0, fstream::end);
	jpeg_size = (unsigned int)in_file.tellg();
	in_file.seekg(0, fstream::beg);
	jpeg_buf = new unsigned char[jpeg_size];
	in_file.read(reinterpret_cast<char*>(jpeg_buf), jpeg_size);
	in_file.close();

	tjhandle decompressor = tjInitDecompress();
	int _width = 0;
	int _height = 0;
	int _subsample = 0;
	int result = tjDecompressHeader2(decompressor, jpeg_buf, jpeg_size,
		&_width, &_height, &_subsample);
	tjDestroy(decompressor);
	*width = _width;
	*height = _height;
	delete[] jpeg_buf;

	if (result != 0) {
		cerr << "[Error] DecodeJpeg: decompress fail " << tjGetErrorStr() << endl;
		return false;
	}

	return true;
}
