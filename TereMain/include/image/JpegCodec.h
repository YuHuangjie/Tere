//
// Created by YuHuangjie on 6/6/2017.
//

#include <turbojpeg.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

/**
 * Decompress a jpeg file.
 *
 * @param filename	The file name
 * @param buf[out]	Decoded byte array. Pixels are stored from bottom to top and 
 *					from left to right (to be compatiable with OpenGL). Each 
 *					pixel if stored in RGB format, where each component takes
 *					8 bits. 
 *					The caller must allocate the memory.
 * @param bufsize	Size of buf (in bytes)
 * @param width		Desired width of decoded image. If width is set to 0, the width of 
 *					the JPEG to be decoded will be used
 * @param height	Desired height of decoded image. If height is set to 0, the height of 
 *					the JPEG to be decoded will be used
 *
 * @return true if decompression succed, false otherwise
 */
inline bool DecodeJpeg(const string &filename, unsigned char *buf, const unsigned int bufsize,
	const unsigned int width, const unsigned int height)
{
    if (buf == nullptr) {
        return false;
    }
	if (bufsize < width * height * 3) {
		string errormsg = string("!![Error] DecodeJpeg: too small buf size: ") + TO_STRING(bufsize);
		throw invalid_argument(errormsg);
	}

    ifstream in_file(filename, fstream::binary);
    unsigned int jpeg_size;
    unsigned char *jpeg_buf = nullptr;

    if (!in_file.is_open()) {
		string errormsg = string("!![Error] DecodeJpeg: open image_file failed: ") + filename;
        throw invalid_argument(errormsg);
    }

    in_file.seekg(0, fstream::end);
    jpeg_size = (unsigned int)in_file.tellg();
    in_file.seekg(0, fstream::beg);
    jpeg_buf = new unsigned char[jpeg_size];
    in_file.read(reinterpret_cast<char*>(jpeg_buf), jpeg_size);
	in_file.close();

    tjhandle decompressor = tjInitDecompress();
    int result = tjDecompress2(decompressor, jpeg_buf, jpeg_size,
                               buf, width, 0, height, TJPF_RGB, TJFLAG_BOTTOMUP);
    tjDestroy(decompressor);
    delete[] jpeg_buf;

    if (result != 0) {
        string error = string("!![Error] DecodeJpeg: decompress fail ") + tjGetErrorStr();
        throw runtime_error(error.c_str());
    }

    return true;
}

/**
* Retrieve information about a JPEG image without decompressing it.
*
* @param filename	The file name
* @param width		A reference to a int variable that will recieve the width (in Pixels)
*					of the JPEG image
* @param height		A reference to a int variable that will recieve the height (in Pixels)
*					of the JPEG image
*
* @return true if decompression succeed, false otherwise
*/
inline bool DecodeJpegHeader(const string &filename, unsigned int &width, unsigned int &height)
{
	ifstream in_file(filename, fstream::binary);
	unsigned int jpeg_size;
	unsigned char *jpeg_buf = nullptr;

	if (!in_file.is_open()) {
		string errormsg = string("!![Error] DecodeJpegHeader: open image_file failed: ") + filename;
		throw invalid_argument(errormsg);
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
	width = _width;
	height = _height;
	delete[] jpeg_buf;

	if (result != 0) {
		string error = string("!![Error] DecodeJpegHeader: decompress fail ") + tjGetErrorStr();
		throw runtime_error(error.c_str());
	}

	return true;
}