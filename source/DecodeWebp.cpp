//
// Created by YuHuangjie on 6/14/2017.
//

#include "Preprocess.h"
#include "Decoder.h"
#include "webp/decode.h"
#include <fstream>
#include <stdexcept>

using namespace std;

bool DecompressWebp(std::string filename, unsigned char *buf, int width, int height)
{
    if (buf == nullptr) {
        return false;
    }

    // open image file
    ifstream in_file(filename, fstream::binary);
    unsigned int webp_size;
    unsigned char *webp_buf = nullptr;

    if (!in_file.is_open()) {
        throw runtime_error("open image_file failed");
    }

    // read raw image data into memory
    in_file.seekg(0, fstream::end);
    webp_size = (unsigned int)in_file.tellg();
    in_file.seekg(0, fstream::beg);
    webp_buf = new unsigned char[webp_size];
    in_file.read(reinterpret_cast<char*>(webp_buf), webp_size);

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

    config.output.colorspace = MODE_RGB;
    config.output.u.RGBA.rgba = buf;
    config.output.u.RGBA.stride = width * 3;
    config.output.u.RGBA.size = width * height * 3;
    config.output.is_external_memory = true;

    if (WebPDecode(webp_buf, webp_size, &config) != VP8_STATUS_OK) {
        delete[] webp_buf;
        webp_buf = nullptr;
        throw runtime_error("WebPDecode failed");
    }
    delete[] webp_buf;
    webp_buf = nullptr;

    return true;
}
