#ifndef RENDER_H
#define RENDER_H

#include "Preprocess.h"
#if GL
#include <GL/glew.h>
#elif GL_ES3 && PLATFORM_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif GL_ES3 && PLATFORM_ANDROID
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

using std::vector;
using std::string;
using glm::mat4;
using glm::vec3;
using glm::ivec4;

class WeightedCamera
{
public:
	WeightedCamera(int i, float w) : index(i), weight(w) {}

	int index;
	float weight;
};

class OBJRender
{
public:
	OBJRender(const LightFieldAttrib &, int fbWidth, int fbHeight);
	OBJRender(const OBJRender &) = delete;
	OBJRender& operator=(const OBJRender &) = delete;
	~OBJRender();

	// render method
	int render();

	// render depth maps
	GLuint AppendDepth(GLuint rgb, unsigned int width, 
		unsigned int height, const mat4 &VP, const mat4 &V);

	// set light field texture
	void SetLightFieldTexs(const vector<GLuint> &lfTexs);

	// set virtual camera pose
	void SetVirtualCamera(const Camera &);

	// set online frame buffer object
	inline void SetDefaultFBO(GLuint fbo) { default_fbo = fbo; }

	// indicate interaction is active
	inline void UseHighTexture(bool flag) { useHighTexture = flag; }

private:
	bool TransferMeshToGL(const string &meshFile);
	bool TransferRefCameraToGL(const vector<mat4> &VP, const vector<mat4> V);
	void SearchInterpCameras(void);
	bool LoadCameraMesh(const string &);

	// compile vertex, fragment shaders
	GLuint LoadShaders(const char * vertex_code, const char * fragment_code);

	int nMeshes;                      // number of objects
	vector<size_t> indexSizes;   // index count in each object
	
	vec3 scene_center;			// center position of scene

	vector<GLuint> vertexArrays;			// VAO
	vector<GLuint> vertexBuffers;          // VBO
	vector<GLuint> elementBuffers;         // EBO

	// default framebuffer object
	GLuint default_fbo;

	// textures storing reference cameas' VP/V matrices
	GLuint ref_cam_VP_texture;
	GLuint ref_cam_V_texture;

	// 
	static const int NUM_INTERP = 12;
	static const int NUM_HIGH_INTERP = 4;

	GLuint scene_program_id;    // shader for multi-view rendering
	GLuint depth_program_id;    // shader for depth rendering

	// attrib/uniform location
	GLint scene_VP_id;
	GLint depth_VP_id;
	GLuint depth_near_location;
	GLuint depth_far_location;
	GLuint scene_near_location;
	GLuint scene_far_location;
	GLuint N_REF_CAMERAS_location;
	GLuint ref_cam_VP_location;
	GLuint ref_cam_V_location;
	GLuint nInterpsLocation;
	GLuint interpIndicesLocation[NUM_INTERP];
	GLuint interpWeightsLocation[NUM_INTERP];
	GLuint lightFieldLocation[NUM_INTERP];

	mat4 Model;		// Model matrix
	mat4 View;			// View matrix
	mat4 Projection;	// Projection matrix

	Camera virtual_camera;  // virtual_camera;

	GLuint frameBuffer;		  // frame buffer for offline rendering
	GLuint renderedTexture;   // texture buffer 
	GLuint depthBuffer;	      // depth buffer

	LightFieldAttrib attrib;  // light field attribute

	// Camera mesh info
	vector< vec3 > camera_vertices;   // camera mesh vertex
	vector< ivec4> camera_quads;      // camera mesh indices

	vector<WeightedCamera> interpCameras;  // interpolating cameras

	vector<vec3> ref_camera_dirs;     // center points to each camera
	vector<float> ref_camera_dists;   // distance between center and cameras
	vector<int> ref_camera_index;     // camera sorted by angle distance
	
	// light field textures (typically low resolution)
	vector<GLuint> lightFieldTexs;    
	// light field textures (typically high resolution)
	GLuint light_field_H[NUM_HIGH_INTERP];

	// indicate using high or low resolution textures
	bool useHighTexture;
};


#endif
