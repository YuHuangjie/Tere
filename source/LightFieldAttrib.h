#ifndef LIGHTFIELDATTRIB_H
#define LIGHTFIELDATTRIB_H

#include <vector>
#include <string>
#include "camera/Camera.h"

struct LightFieldAttrib
{
	LightFieldAttrib() :
		N_REF_CAMERAS(0),
		ref_cameras(),
		ref_cameras_VP(),
		ref_cameras_V(),
		obj_file(),
		ref_camera_center(),
		ref_camera_radius(0.f),
		camera_mesh_name(),
		glnear(0.f),
		glfar(0.f),
		image_list(),
		width_H(0),
		height_H(0),
		width_L(0),
		height_L(0)
	{	}

	int N_REF_CAMERAS;	            // number of reference cameras

	std::vector<Camera> ref_cameras;	     // reference cameras
	std::vector<glm::mat4x4> ref_cameras_VP; // ref cameras' View-projection
	std::vector<glm::mat4x4> ref_cameras_V;  // ref cameras' View matrix

	std::string obj_file;			// object mesh

	glm::vec3 ref_camera_center;    // ref cameras' center
	float ref_camera_radius;        // ref cameras' radius
	std::string camera_mesh_name;   // ref camera mesh

	float glnear;    // near plane
	float glfar;     // far plane

	std::vector<std::string> image_list;

	unsigned int width_H, height_H;   // high resolution image size
	unsigned int width_L, height_L;   // low resolution image size
};

#endif