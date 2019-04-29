#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <cstdint>

void BoundingBoxCPU(const float *v, const size_t szV, float &xmin, float &xmax,
	float &ymin, float &ymax, float &zmin, float &zmax);
void BoundingBoxGPU(const float *v, const size_t szV, float &xmin, float &xmax,
	float &ymin, float &ymax, float &zmin, float &zmax);



#endif /* BOUNDING_BOX_H */