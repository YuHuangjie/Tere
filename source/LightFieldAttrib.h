#ifndef LIGHTFIELDATTRIB_H
#define LIGHTFIELDATTRIB_H

#include <vector>
#include <string>
#include "RenderCamView.h"

struct LightFieldAttrib
{
	LightFieldAttrib() {
		N_REF_CAMERAS = 0;
		N_REF_CAMERAS_HIGH = 0;
		N_REF_CAMERAS_LOW = 0;
		light_field_tex = nullptr;
		glnear = 0.f;
		glfar = 0.f;
		width_H = 0;
		height_H = 0;
		width_L = 0;
		height_L = 0;
	}

	int N_REF_CAMERAS;	// number of reference cameras
	int N_REF_CAMERAS_HIGH;	// number of high resolution reference cams
	int N_REF_CAMERAS_LOW;	// number of low resolution reference cams
	unsigned int *light_field_tex;	// light field image texture array

	//std::string depth_vs_file;
	//std::string depth_frag_file;
	//std::string scene_vs_file;
	//std::string scene_frag_file;

	std::vector<RenderCamView> ref_cameras;	// reference cameras
	std::vector<glm::mat4x4> ref_cameras_VP;
	std::vector<glm::mat4x4> ref_cameras_V;

	std::string obj_file;

	glm::vec3 ref_camera_center;
	float ref_camera_radius;
	std::string camera_mesh_name;

	float glnear;
	float glfar;

	std::string image_file_prefix;
	std::vector<std::string> image_list;

	unsigned int width_H, height_H;
	unsigned int width_L, height_L;
};

#endif