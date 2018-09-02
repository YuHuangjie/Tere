#ifndef RENDERER_H
#define RENDERER_H

#include "common/Common.hpp"
#if GL_WIN || GL_OSX
#include <GL/glew.h>
#elif GL_ES3_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif GL_ES3_ANDROID
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <string>
#include <thread>

#include "LightFieldAttrib.h"
#include "WeightedCamera.h"

using std::vector;
using std::string;
using glm::mat4;
using glm::vec3;
using glm::ivec4;

class Renderer
{
public:
	Renderer(const LightFieldAttrib &, const size_t fbw, const size_t fbh);
	Renderer(const Renderer &) = delete;
	Renderer& operator=(const Renderer &) = delete;
	~Renderer();

	// render method
	int Render(const vector<int> &viewport);
    
    // Replace high resolution textures
    void ReplaceHighTexture();

	// render depth maps
	GLuint AppendDepth(GLuint rgb, unsigned int width, 
		unsigned int height, const mat4 &VP, const mat4 &V);

	// set light field texture
	void SetLightFieldTexs(const vector<GLuint> &lfTexs);

	// Set interpolation cameras
	void ClearInterpCameras();
	bool AddInterpCameras(const WeightedCamera &camera);

	// set virtual camera 
	void SetVirtualCamera(const Camera &);

	// indicate interaction is active
	inline void UseHighTexture(bool flag) { useHighTexture = flag; }

private:
	bool TransferMeshToGL(const string &meshFile);
	bool TransferRefCameraToGL(const vector<mat4> &VP, const vector<mat4> V);

	int nMeshes;                      // number of objects
	vector<size_t> indexSizes;   // index count in each object
	
	vector<GLuint> vertexArrays;			// VAO
	vector<GLuint> vertexBuffers;          // VBO
	vector<GLuint> elementBuffers;         // EBO
	vector<GLuint> vColorBuffers;			// vertex color buffer

	// textures storing reference cameas' VP/V matrices
	GLuint ref_cam_VP_texture;
	GLuint ref_cam_V_texture;

	//
    enum {
        NUM_INTERP = 12,
        NUM_HIGH_INTERP = 4,
    };

	GLuint scene_program_id;    // shader for multi-view rendering
	GLuint depth_program_id;    // shader for depth rendering

	// attrib/uniform location
	GLint scene_VP_id;
	GLint depth_VP_id;
	int depth_near_location;
	int depth_far_location;
	int scene_near_location;
	int scene_far_location;
	int N_REF_CAMERAS_location;
	int ref_cam_VP_location;
	int ref_cam_V_location;
	int nInterpsLocation;
	int interpIndicesLocation[NUM_INTERP];
	int interpWeightsLocation[NUM_INTERP];
	int lightFieldLocation[NUM_INTERP];

	mat4 Model;		// Model matrix
	mat4 View;			// View matrix
	mat4 Projection;	// Projection matrix

	Camera virtual_camera;  // virtual_camera;

	// frame buffer for depth appending to normal rgb texture
	GLuint _rgbdFrameBuffer;
	GLuint _rgbdDepthBuffer;

	// frame buffer for rendering results
	GLuint _finalFrameBuffer;
	GLuint _finalRenderTex;
	GLuint _finalDepthBuffer;

	LightFieldAttrib attrib;  // light field attribute

	vector<WeightedCamera> interpCameras;  // interpolating cameras
	
	// light field textures (typically low resolution)
	vector<GLuint> lightFieldTexs;    
	// light field textures (typically high resolution)
	GLuint light_field_H[NUM_HIGH_INTERP];

	// indicate using high or low resolution textures
	bool useHighTexture;
};


#endif
