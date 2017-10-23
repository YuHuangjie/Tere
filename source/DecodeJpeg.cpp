//
// Created by YuHuangjie on 6/6/2017.
//

#include "Decoder.h"
#include "turbojpeg.h"
#include <fstream>
#include <stdexcept>

using namespace std;

namespace {
}

bool DecompressJpeg(string filename, unsigned char *buf, int width, int height)
{
    if (buf == nullptr) {
        return false;
    }

    ifstream in_file(filename, fstream::binary);
    unsigned int jpeg_size;
    unsigned char *jpeg_buf = nullptr;

    if (!in_file.is_open()) {
        throw runtime_error("open image_file failed");
    }

    in_file.seekg(0, fstream::end);
    jpeg_size = (unsigned int)in_file.tellg();
    in_file.seekg(0, fstream::beg);
    jpeg_buf = new unsigned char[jpeg_size];
    in_file.read(reinterpret_cast<char*>(jpeg_buf), jpeg_size);

    tjhandle decompressor = tjInitDecompress();
    int result = tjDecompress2(decompressor, jpeg_buf, jpeg_size,
                               buf, width, 0, height, TJPF_RGB, TJFLAG_BOTTOMUP);
    tjDestroy(decompressor);
    delete[] jpeg_buf;

    if (result != 0) {
        string error = string("decompress fail ") + tjGetErrorStr();
        throw runtime_error(error.c_str());
    }

    return true;
}
