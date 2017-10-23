#ifndef RENDER_H
#define RENDER_H

#include "Preprocess.h"
#ifdef GL
#include <GL/glew.h>
#elif defined GL_ES3 && defined IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined GL_ES3 && defined __ANDROID__
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "tinyobj.h"
#include <vector>
#include <string>
#include <thread>

#include "LightFieldAttrib.h"
#include "ClosestCamera.h"

const int CLOSEST_CAMERA_NR = 4;

class OBJRender
{
public:
	OBJRender(std::string obj_name, int w, int h, 
		std::string camera_mesh_name);
	~OBJRender();

	// render method
	int render();

	// render depth maps
	void RenderDepth(int id, unsigned char *pixels, unsigned width, unsigned height);
	GLuint RenderRGBD(int id, GLuint rgb, unsigned int width, unsigned int height);

	// set virtual camera pose
	void SetVirtualCamera(RenderCamView);
	void SetMatrices(const glm::mat4 &_View, 
		const glm::mat4 &_Projection, const glm::mat4 &_Model=glm::mat4(1.0));
	void SetRenderLocation(const glm::vec3 &loc);
	void SetLightFieldAttrib(const LightFieldAttrib&);
	inline void SetDefaultFBO(GLuint fbo) { default_fbo = fbo; }
	void SetRenderRes(GLuint width, GLuint height);

	// get pointer of texture bound with tex
	void GetTextureData(GLuint tex, unsigned char * image);
	glm::vec3 GetSceneCenter(void);

	// Transfer lookup table to OpenGL
	void TransferLookupTableToGL(const std::vector<int> &table);

	// indicate interaction is active
	inline void SetFlag(bool interacting) { this->interacting = interacting; }

	// get values stored in 'camera_set', 'ind_camera_set', 'dind_camera_set'
	std::vector<int> GetSelectedCamera(void);

private:
	void TransferMeshToGL(void);
	void TransferRefCameraToGL(void);
	glm::vec3 CalcObjCenter(void);
	void LocateClosestCameras(void);
	void LoadCameraMesh(const std::string &);

	// compile vertex, fragment shaders
	GLuint LoadShaders(const char * vertex_code, const char * fragment_code);

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	glm::vec3 scene_center;			// center position of scene

	GLuint *VertexArray;			// VAO
	GLuint *vertexbuffers;
	GLuint *normalbuffers;
	GLuint *texelbuffers;
	GLuint *elementbuffers;
	GLuint lookup_table_buffer;		// SSBO for lookup table
	GLuint ref_cam_MVP_buffer;		// SSBO for reference cameras' VP
	GLuint ref_cam_MV_buffer;		// SSBO for reference cameras' V

	int targetWidth, targetHeight;

	// default framebuffer object
	GLuint default_fbo;

	// textures storing reference cameas' VP/V matrices
	GLuint ref_cam_VP_texture;
	GLuint ref_cam_V_texture;

	/* attrib/uniform location */
	GLuint ref_cam_VP_location;
	GLuint ref_cam_V_location;
	GLuint scene_program_id;
	GLuint depth_program_id;
	GLint scene_VP_id;
	GLint depth_VP_id;
	GLint depth_V_id;
	GLuint depth_near_location;
	GLuint depth_far_location;
	GLuint scene_near_location;
	GLuint scene_far_location;
	GLuint N_REF_CAMERAS_location;
	GLuint N_REF_CAMERAS_HIGH_location;
	GLuint N_REF_CAMERAS_LOW_location;
	GLuint WEIGHTS_location;
	GLuint INDIRECT_WEIGHTS_location;
	GLuint DINDIRECT_WEIGHTS_location;
	GLuint CAMERAS_location;
	GLuint DINDIRECT_CAMERAS_location;
	GLuint INDIRECT_CAMERAS_location;
	GLuint light_field_location[CLOSEST_CAMERA_NR];
	GLuint T_light_field_location[CLOSEST_CAMERA_NR];
	GLuint T2_light_field_location[CLOSEST_CAMERA_NR];

	glm::mat4 Model;		// Model matrix
	glm::mat4 View;			// View matrix
	glm::mat4 Projection;	// Projection matrix
	glm::vec3 render_camera_location;	// camera position

	RenderCamView virtual_camera;

	GLuint FramebufferName[1];		// Frame buffer
	GLuint renderedTexture[1];		// store depth image
	GLuint depthrenderbuffer[1];	// opengl inner depth test

	LightFieldAttrib attrib;
	ClosestCameraSet camera_set;
	ClosestCameraSet ind_camera_set;
	ClosestCameraSet dind_camera_set;
	std::vector<int> lookup_table;
	std::vector<glm::vec3> ref_camera_dirs;
	std::vector<float> ref_camera_dists;
	std::vector<int> ref_camera_index;

	/* camera mesh info*/
	std::vector< glm::vec3 > camera_vertices;
	std::vector< glm::ivec4> camera_quads;
	bool interacting;		// indicate whether user is interacting via mouse/finger
	GLuint light_field_H[CLOSEST_CAMERA_NR];	// texture for high resolution images
};


#endif
