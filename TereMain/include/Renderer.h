#ifndef RENDERER_H
#define RENDERER_H
#include <vector>
#include <string>
#include <thread>
#include <memory>

#include "glm/glm.hpp"
#include "WeightedCamera.h"
#include "GLHeader.h"
#include "TereScene.h"

#ifdef USE_CUDA
#include "cuda_gl_interop.h"
#endif

using std::vector;
using std::string;
using std::shared_ptr;

class Renderer
{
public:
	Renderer(shared_ptr<TereScene> scene);
	~Renderer();

	// Inform updated geometry in _scene
	bool UpdatedGeometry();	

	// Inform updated images in _scene
	bool UpdatedLF();

	// render method
	int Render(const vector<int> &viewport);

	// Set interpolation cameras
	void ClearInterpCameras();
	bool AddInterpCameras(const WeightedCamera &camera);

	// set virtual camera 
	void SetViewer(const glm::mat4 &M, const glm::mat4 &V, const glm::mat4 &P);

private:
	bool RefreshDepth();

private:
	enum {
		FRAMEBUFFER_WIDTH = 1024,	// width of rendered texture
		FRAME_BUFFER_HEIGHT = 1024,	// height of rendered texture
		NUM_INTERP = 12,			// maximum interp camera counts
	};

	shared_ptr<TereScene> _scene;

	GLuint _sceneShader;			// shader for multi-view rendering
	GLuint _depthShader;			// shader for depth rendering

	GLuint _VPTexture;				// view-proj mats are stored as texture
	GLuint _VTexture;				// view mats are stored as texture

	/* depth shader uniform locations */
	GLint _dVPLct;					// view-proj matrix of render cam
	GLint _dNearLct;				// near
	GLint _dFarLct;					// far

	/* scene shader uniform locations */
	GLint _sNearLct;				// near 
	GLint _sFarLct;					// far 
	GLint _sNCamLct;				// number of reference cameras
	GLuint _sVPLct;					// view-proj matrix of render cam
	GLint _sVPRefLct;				// view-proj matices texture
	GLint _sVRefLct;				// view matrices texture
	GLint _sNInterpLct;				// number of interpolation cameras
	GLint _sItpIdLct[NUM_INTERP];	// indices of interpolation cameras
	GLint _sItpWtLct[NUM_INTERP];	// weights of interpolation cameras
	GLint _sLFLct[NUM_INTERP];		// light field texture

	glm::mat4 _model;				// model mat of render camera
	glm::mat4 _view;				// view mat of render camera
	glm::mat4 _proj;				// projection mat of render camera

	vector<GLuint> _rgbTextures;	// each ref camera has a texture
	vector<GLuint> _rgbdTextures;	// rgb texture + depth
									
	GLuint _fbo;					// scene's frame buffer
	GLuint _dAttach;				// scene's depth attachment
	GLuint _cAttach;				// scene's color attachment

	GLuint _rgbdFbo;				// depth's frame buffer
	GLuint _rgbdDAttach;			// depth's depth attachment

	vector<size_t> indexSizes;		// index count in each object
	
	GLuint _VAO;					// VAO
	GLuint _posBuffer;				// vertex position buffer
	//GLuint _clrBuffer;			// vertex color buffer
	GLuint _elmBuffer;				// element buffer
	GLuint _PBO;					// PBO (for unpacking to texture)

#ifdef USE_CUDA
	cudaGraphicsResource* _cuPosBuffer;	// cuda resource bound on _posBuffer
	cudaGraphicsResource* _cuElmBuffer;	// cuda resource bound on _elmBuffer
	cudaGraphicsResource* _cuPBO;		// cuda resource bound on _PBO
#endif

	bool _refreshDepth;				// require updating depth

	vector<WeightedCamera> _interpCams;	// interpolation cameras
};

#endif /* RENDERER_H */
