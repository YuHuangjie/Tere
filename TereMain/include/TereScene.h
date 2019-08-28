#ifndef TERE_SCENE_H
#define TERE_SCENE_H

#include <vector>
#include <string>
#include <array>
#include "Type.h"
#include "camera/Intrinsic.hpp"
#include "camera/Extrinsic.hpp"

/* Describe the setting of the scene */
struct TereScene
{
	/**************************************************************************
	*							Scene constraints
	*************************************************************************/

	// number of reference cameras
	size_t nCams;

	// render mode
	RENDER_MODE rmode;

	// draw mode
	bool dArray;
	bool dElement;
		
	float glnear;					// near plane
	float glfar;					// far plane

	// Only used in LINEAR mode. 
	size_t rows;					// Rows of reference cameras

	// Used in ALL and SPHERE mode
	glm::vec3 center;				// mesh bounding box center
	float radius;					// average distance from center to cameras

	/**************************************************************************
	*							Scene data
	*************************************************************************/

	// vertex buffer
	float *v;

	// vertex buffer size
	size_t szVBuf;

	// in-use vertex buffer size
	size_t szV;

	// face buffer
	int *f;

	// face buffer size
	size_t szFBuf;

	// in-use face buffer size
	size_t szF;

	// indicate memory location of geometry (either CUDA or CPU)
	bool GPU;

	// camera parameters
	std::vector< Intrinsic > intrins;
	std::vector< Extrinsic > extrins;

	// reference image data
	int width;
	int height;
	std::vector< uint8_t* > rgbs;

	/**************************************************************************
	*							Methods
	*************************************************************************/
	TereScene();
	TereScene(const size_t n);
	TereScene(const TereScene&) = delete;
	TereScene& operator=(const TereScene&) = delete;

	bool UpdateGeometry(const float *v, const size_t szV, const int *f,
		const size_t szF, bool GPU);

	bool UpdateImage(const size_t id, const uint8_t *data, const int w,
		const int h);

	bool UpdateCamera(const size_t id, const std::array<float, 9> &K, 
		const std::array<float, 16> &M, bool w2c, bool yIsUp);

	bool Configure();
};

#endif /* TERE_SCENE_H */
