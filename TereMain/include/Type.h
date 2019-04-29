#ifndef TYPE_H
#define TYPE_H

#include <vector>

enum UIType : unsigned int
{
	MOVE = 0,
	TOUCH = 1,
	LEAVE = 2,
};

enum RENDER_MODE : unsigned int
{
	LINEAR = 0,
	SPHERE = 1,
	ALL = 2,
};

// Get image size information (usually rechived by decoding image header)
typedef bool(*DecHeaderFunc)(const char *file, int *width, int *height);

// Decode image and store RGB buffer into *buf* (outWidth or outHeight might
// be difference from image's original size)
typedef bool(*DecImageFunc)(const char *file, const int outWidth, 
	const int outHeight, void *buf, const size_t sz);

#endif /* TYPE_H */
