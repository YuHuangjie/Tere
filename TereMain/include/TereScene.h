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

//struct LightFieldAttrib
//{
//	LightFieldAttrib() :
//		N_REF_CAMERAS(0),
//		ref_cameras(),
//		ref_cameras_VP(),
//		ref_cameras_V(),
//		obj_file(),
//		ref_camera_center(),
//		ref_camera_radius(0.f),
//		glnear(0.f),
//		glfar(0.f),
//		image_list(),
//		width_H(0),
//		height_H(0),
//		width_L(0),
//		height_L(0),
//		_uiMode(""),
//		camera_mesh_name(""),
//		_rows(0)
//	{	}
//
//	int N_REF_CAMERAS;	            // number of reference cameras
//
//	std::vector<Camera> ref_cameras;	     // reference cameras
//	std::vector<glm::mat4x4> ref_cameras_VP; // ref cameras' View-projection
//	std::vector<glm::mat4x4> ref_cameras_V;  // ref cameras' View matrix
//
//	std::string obj_file;			// object mesh
//
//	glm::vec3 ref_camera_center;    // ref cameras' center
//	float ref_camera_radius;        // ref cameras' radius
//
//	float glnear;    // near plane
//	float glfar;     // far plane
//
//	std::vector<std::string> image_list;
//
//	unsigned int width_H, height_H;   // high resolution image size
//	unsigned int width_L, height_L;   // low resolution image size
//
//	std::string _uiMode;
//	std::string camera_mesh_name;   // ref camera mesh (_uimode="arcball")
//	size_t _rows;		// linear rows (_uimode="linear")
//};

#endif /* TERE_SCENE_H */
