#include <thrust/transform_reduce.h>
#include <thrust/device_vector.h>
#include <thrust/pair.h>
#include <thrust/random.h>
#include <thrust/extrema.h>

#include "BoundingBox.h"
#include "Const.h"

struct point3d
{
	float x, y, z;

	__host__ __device__
		point3d() : x(0), y(0), z(0) {}

	__host__ __device__
		point3d(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct bbox
{
	point3d lower_left, upper_right;

	// --- Empty box constructor
	__host__ __device__ bbox() {}

	// --- Construct a box from a single point
	__host__ __device__ bbox(const point3d &point) 
		: lower_left(point), upper_right(point) {}

	// construct a box from a single point
	__host__ __device__	bbox& operator=(const point3d &point)
	{
		lower_left = point;
		upper_right = point;
		return *this;
	}

	// --- Construct a box from a pair of points
	__host__ __device__ bbox(const point3d &ll, const point3d &ur)
		: lower_left(ll), upper_right(ur) {}
};

// --- Reduce a pair of bounding boxes (a, b) to a bounding box containing a and b
struct bbox_reduction : public thrust::binary_function<bbox, bbox, bbox>
{
	__host__ __device__ bbox operator()(bbox a, bbox b)
	{
		// --- Lower left corner
		point3d ll(thrust::min(a.lower_left.x, b.lower_left.x),
			thrust::min(a.lower_left.y, b.lower_left.y), 
			thrust::min(a.lower_left.z, b.lower_left.z));

		// --- Upper right corner
		point3d ur(thrust::max(a.upper_right.x, b.upper_right.x),
			thrust::max(a.upper_right.y, b.upper_right.y), 
			thrust::max(a.upper_right.z, b.upper_right.z));

		return bbox(ll, ur);
	}
};

void BoundingBoxGPU(const float *v, const size_t szV, float &xmin, float &xmax,
	float &ymin, float &ymax, float &zmin, float &zmax)
{
	const size_t N = szV / BYTES_PER_VERTEX;
	//thrust::device_vector<point3d> points(N);
	thrust::device_ptr<point3d> points((point3d*)v);

	// --- The initial bounding box contains the first point of the point cloud
	bbox init = bbox(points[0], points[0]);
	
	// --- Binary reduction operation
	bbox_reduction binary_op;

	// --- Compute the bounding box for the point set
	bbox result = thrust::reduce(points, points + N, init, binary_op);

	xmin = result.lower_left.x;
	xmax = result.upper_right.x;
	ymin = result.lower_left.y;
	ymax = result.upper_right.y;
	zmin = result.lower_left.z;
	zmax = result.upper_right.z;
}