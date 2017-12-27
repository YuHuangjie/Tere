#ifndef LFLoader_H
#define LFLoader_H

#include "Preprocess.h"
#if GL
#include <GL\glew.h>
#elif GL_ES3 && PLATFORM_IOS
#include <OpenGLES/ES3/gl.h>
#elif GL_ES3 && PLATFORM_ANDROID
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

	vector<GLuint> PostDecompress(void);
	void Decompress(const int no_thread);
	vector<GLuint> GenerateRGBDTextures(const vector<GLuint> &rgbs,
		const unique_ptr<OBJRender> &renderer);

	inline LightFieldAttrib& GetLightFieldAttrib(void) { 
		return attrib;
	};

private:
	void OnDecompressing(const int thread_id, const int thread_nr);

	// light field configuration
	LightFieldAttrib attrib;

	// buffer for RGB texture
	vector<uint8_t> rgbBuffer;
};


#endif
