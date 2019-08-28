#include <limits>

#include "BoundingBox.h"

void BoundingBoxCPU(const float *v, const size_t szV, float &xmin, float &xmax,
	float &ymin, float &ymax, float &zmin, float &zmax)
{
	float minx = std::numeric_limits<float>::max();
	float maxx = -std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max();
	float maxy = -std::numeric_limits<float>::max();
	float minz = std::numeric_limits<float>::max();
	float maxz = -std::numeric_limits<float>::max();
	float x = 0.f, y = 0.f, z = 0.f;
	const float *pv = v;
	const float * const pvend = v + (szV / sizeof(float));
	
	while (pv < pvend) {
		x = *pv++;
		y = *pv++;
		z = *pv++;
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
		if (z < minz) minz = z;
		if (z > maxz) maxz = z;
	}

	xmin = minx;
	xmax = maxx;
	ymin = miny;
	ymax = maxy;
	zmin = minz;
	zmax = maxz;
}