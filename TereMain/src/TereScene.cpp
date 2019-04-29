#include <stdexcept>
#include <chrono>
#include <numeric>

#include "TereScene.h"
#include "Const.h"
#include "Memory.h"
#include "Error.h"
#include "BoundingBox.h"

using namespace std;

TereScene::TereScene()
	:
	nCams(0),
	rmode(RENDER_MODE::ALL),
	dArray(false),
	dElement(false),
	glnear(50.f),
	glfar(100.f),
	rows(0),
	center(0.f),
	radius(0.f),
	v(nullptr),
	szVBuf(MAX_VERTEX * BYTES_PER_VERTEX),
	szV(0),
	f(nullptr),
	szFBuf(MAX_FACE * BYTES_PER_FACE),
	szF(0),
	GPU(false),
	width(0),
	height(0)
{}

TereScene::TereScene(const size_t n)
	: TereScene()
{
	nCams = n;
	intrins = vector< Intrinsic >(n);
	extrins = vector< Extrinsic >(n);
	rgbs = vector< uint8_t* >(n, nullptr);
}

static void Copy(void *dst, const void *src, const size_t sz, bool fromcuda, bool tocuda)
{
	if (!dst || !src) return;

	if (fromcuda && tocuda) 
		MEMCPY(dst, src, sz, MEMCPY_DEV2DEV);	// device to device
	else if (fromcuda && !tocuda)
		MEMCPY(dst, src, sz, MEMCPY_DEV2HOST);	// device to host
	else if (!fromcuda && tocuda) 
		MEMCPY(dst, src, sz, MEMCPY_HOST2DEV);	// host to device
	else 
		MEMCPY(dst, src, sz, MEMCPY_HOST2HOST);	// host to host
}

bool TereScene::UpdateGeometry(const float * _v, const size_t _szV, const int * _f,
	const size_t _szF, bool _GPU)
{
	if (_szV > szVBuf) {
		RETURN_ON_ERROR("Too many vertices");
	}
	if (_szF > szFBuf) {
		RETURN_ON_ERROR("Too many faces");
	}

	// allocate memory for vertices and faces
#ifdef USE_CUDA
	if (_v && !v) CUDA_ERR_CHK(cudaMalloc(&v, szVBuf));
	if (_f && !f) CUDA_ERR_CHK(cudaMalloc(&f, szFBuf));
	GPU = true;
#else
	if (_v && !v) v = reinterpret_cast<float*>(new uint8_t[szVBuf]);
	if (_f && !f) f = reinterpret_cast<int*>(new uint8_t[szFBuf]);
	GPU = false;
#endif

	// copy vertices and faces
	try {
		if (_v) {
			Copy(v, _v, _szV, _GPU, GPU);
			szV = _szV;
		}
		if (_f) {
			Copy(f, _f, _szF, _GPU, GPU);
			szF = _szF;
		}
	}
	catch (std::exception &e) {
		RETURN_ON_ERROR(e.what());
	}

	// Set drawing mode to glDrawArray if face is NULL. Otherwise, set it to 
	// glDrawElement.
	dArray = (_f == nullptr);
	dElement = !dArray;

	return true;
}

bool TereScene::UpdateImage(const size_t _id, const uint8_t * _data, const int _w,
	const int _h)
{
	if (_id < 0 || _id >= nCams) {
		RETURN_ON_ERROR("Invalid camera index");
	}
	if (!_data) {
		RETURN_ON_ERROR("data is NULL");
	}
	if ((width != 0 && _w != width) || _w < 1) {
		RETURN_ON_ERROR("Invalid width: %d", _w);
	}
	if ((height != 0 && _h != height) || _h < 1) {
		RETURN_ON_ERROR("Invalid height: %d", _h);
	}

	if (width == 0 || height == 0) {
		width = _w;
		height = _h;
	}

	// allocate image memory
	if (rgbs[_id] == nullptr) {
#ifdef USE_CUDA
		CUDA_ERR_CHK(cudaMalloc(&rgbs[_id], _w * _h * 3));
		GPU = true;
#else
		rgbs[_id] = new uint8_t[_w * _h * 3]();
		GPU = false;
#endif 
	}

	// copy image
	try {
		Copy(rgbs[_id], _data, _w * _h * 3, false, GPU);
	}
	catch (std::exception &e) {
		RETURN_ON_ERROR(e.what());
	}
	return true;
}

bool TereScene::UpdateCamera(const size_t id, const array<float, 9> &K,
	const array<float, 16> &M, bool w2c, bool yIsUp)
{
	if (id < 0 || id >= nCams) {
		RETURN_ON_ERROR("Invalid camera index");
	}

	intrins[id] = Intrinsic(K.data());
	extrins[id] = Extrinsic(M.data(), w2c, yIsUp);
	return true;
}

#define TEST(t) { if (!(t)) RETURN_ON_ERROR(#t); }

static float CalculateRadius(const vector<Extrinsic> &extrins, const array<float, 3> &center)
{
	vector<float> dists(extrins.size());
	glm::vec3 c(center[0], center[1], center[2]);

	for (size_t i = 0; i < extrins.size(); ++i) {
		glm::vec3 pcam = extrins[i].Pos();
		dists[i] = glm::length(pcam - c);
	}

	float radius = std::accumulate(dists.cbegin(), dists.cend(), 0.f) / dists.size();
	return radius;
}

static void FindBBCenterAndRadius(const float *v, const size_t szV, glm::vec3 &center, float &radius)
{
	float minx = 0.f, maxx = 0.f, miny = 0.f, maxy = 0.f, minz = 0.f, maxz = 0.f;

	auto start = chrono::high_resolution_clock::now();
#ifdef USE_CUDA
	BoundingBoxGPU(v, szV, minx, maxx, miny, maxy, minz, maxz);
#else
	BoundingBoxCPU(v, szV, minx, maxx, miny, maxy, minz, maxz);
#endif /* USE_CUDA */
	auto end = chrono::high_resolution_clock::now();

	center.x = (minx + maxx) / 2;
	center.y = (miny + maxy) / 2;
	center.z = (minz + maxz) / 2;

	radius = glm::length(center - glm::vec3(minx, miny, minz));
}

static float CalcCameraRadius(const vector<Extrinsic> &extrins, const glm::vec3 center)
{
	float distAdds = 0.f;

	for (size_t i = 0; i < extrins.size(); ++i) {
		distAdds += glm::length(center - extrins[i].Pos());
	}

	return distAdds / extrins.size();
}

bool TereScene::Configure()
{
	/* Check validity of all variables */
	TEST(nCams > 0);
	TEST(rmode >= RENDER_MODE::LINEAR && rmode <= RENDER_MODE::ALL);
	TEST(rmode != RENDER_MODE::LINEAR || rows > 0);
	TEST(!dElement || f);
	TEST(!dArray || !f);
	TEST(v);
	TEST(szVBuf > 0);
	TEST(szV > 0);

	// face can be empty

	// trivial tests
	for (auto intrin : intrins) TEST(abs(intrin.cx) > 1e-5);		
	for (auto extrin : extrins) TEST(abs(extrin.Right()[0]) > 1e-5);

	for (auto rgb : rgbs) TEST(rgb);
	TEST(width > 0 && height > 0);

	// Calculate mesh bounding box and near/far range
	float boxRadius = 0.f;
		
	FindBBCenterAndRadius(v, szV, center, boxRadius);
	radius = CalcCameraRadius(extrins, center);
	glnear = radius - boxRadius;
	glfar = radius + boxRadius;

	return true;
}