#ifndef LFLoader_H
#define LFLoader_H

#include "common/Common.hpp"
#if GL
#include <GL\glew.h>
#elif GL_ES3_IOS
#include <OpenGLES/ES3/gl.h>
#elif GL_ES3_ANDROID
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
#include "Renderer.h"

using std::string;
using std::vector;
using std::map;
using std::unique_ptr;
using std::runtime_error;
using std::istringstream;
using std::pair;
using std::ifstream;
using std::getline;

class LFLoader
{
public:
	explicit LFLoader(const string &profile);
	LFLoader(const LFLoader &) = delete;
	LFLoader& operator=(const LFLoader&) = delete;

	void LoadProfile(const string &profile_prefix, 
		const map<string, string> &profile);

	bool Decompress(const int no_thread);
	vector<GLuint> GenerateRGBDTextures(const unique_ptr<Renderer> &);

	inline LightFieldAttrib& GetLightFieldAttrib(void) { 
		return attrib;
	};

private:
	bool OnDecompressing(const int thread_id, const int thread_nr);

	// light field configuration
	LightFieldAttrib attrib;

	// buffer for RGB texture
	vector<uint8_t> rgbBuffer;
};


#endif
