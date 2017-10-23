#pragma once
#ifndef LFLoader_H
#define LFLoader_H

#include "Preprocess.h"
#ifdef GL
#include <GL\glew.h>
#elif defined GL_ES3 && defined IOS
#include <OpenGLES/ES3/gl.h>
#elif defined GL_ES3 && defined __ANDROID__
#include <GLES3/gl31.h>
#endif

#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <map>
#include <memory>


#include "LightFieldAttrib.h"
#include "OBJRender.h"

using namespace std;

class LFLoader
{
public:
	LFLoader(const string &profile);
	~LFLoader();

	void LoadProfile(void);
	void LoadPreset(void);

	void PreDecompress(void);
	void PostDecompress(void);
	void Decompress(const int no_thread);
	void GenerateRGBDTexture(const unique_ptr<OBJRender> &renderer);

	inline int GetWidth(void) {	return image_width_H;	}
	inline int GetHeight(void) { return image_height_H; }
	inline LightFieldAttrib& GetLightFieldAttrib(void) { return light_field_attrib; };
	inline vector<int>& GetLookupTable(void) { return lookup_table; }
	inline glm::vec3 GetRefCameraCenter(void) { return ref_camera_center; }
	inline float GetRefCameraRadius(void) { return ref_camera_radius; }
	inline GLfloat GetNear(void) { return glnear; }
	inline GLfloat GetFar(void) { return glfar; }

private:
	void OnDecompressing(const int thread_id, const int thread_nr);

	// light field configuration
	LightFieldAttrib light_field_attrib;
	map<string, string> profile;

	string profile_prefix;
	string image_file_prefix;
	vector<string> image_files;
	int image_width_H, image_height_H;	// high resolution width, height
	int image_width_L, image_height_L;	// low resolution width, height
	size_t image_num_high_res;	// high resolution image number
	size_t image_num_low_res;	// low resolution image number

	// reference fitted sphere
	glm::vec3 ref_camera_center;
	float ref_camera_radius;

	// lookup_table
	vector<int> lookup_table;

	// near/far plane
	GLfloat glnear, glfar;

	// buffer for RGB texture
	unsigned char *rgb_buffer;
};


#endif
