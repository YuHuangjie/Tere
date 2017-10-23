#ifndef DECODER_H
#define DECODER_H

#include <string>

bool Decompress(std::string filename, unsigned char *buf, int width, int height);
bool DecompressJpeg(std::string filename, unsigned char *buf, int width, int height);
bool DecompressWebp(std::string filename, unsigned char *buf, int width, int height);

#endif
