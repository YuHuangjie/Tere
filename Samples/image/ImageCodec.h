#ifndef IMAGE_CODEC_H
#define IMAGE_CODEC_H

#include <cstdint>

/******************************************************************************
 *				Customized decoder
 *****************************************************************************/
bool ImageHeaderDecoder(const char *filename, int *width, int *height);

bool ImageDecoder(const char *filename, const int width, const int height,
	void *buf, const size_t bufsize);

/******************************************************************************
 *				JPEG decoder
 *****************************************************************************/
bool JpegHeaderDecoder(const char *filename, int *width, int *height);

bool JpegDecoder(const char *filename, const int width, const int height,
	void *buf, const size_t bufsize);


/******************************************************************************
 *				WEBP decoder
 *****************************************************************************/
bool WebpHeaderDecoder(const char *filename, int *width, int *height);

bool WebpDecoder(const char *filename, const int width, const int height,
	void *buf, const size_t bufsize);


#endif /* IMAGE_CODEC_H */
