#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <list>

#include "glm/gtc/type_ptr.hpp"
#include "Renderer.h"
#include "shader/renderer_frag.h"
#include "shader/renderer_vs.h"
#include "shader/depth_frag.h"
#include "shader/depth_vs.h"

#include "RenderUtils.h"
#include "Error.h"
#include "Const.h"
#include "ToString.h"
#include "camera/Intrinsic.hpp"
#include "camera/Extrinsic.hpp"

#ifdef USE_CUDA
#include "CudaUtils.h"
#endif

using namespace glm;
using namespace std;

// generate textures for storing view-proj matrices and view matrices.
static bool TransmitCameraToGL(const vector<Intrinsic> ins, const vector<Extrinsic> exs,
	const float w, const float h, const float near, const float far,
	GLuint &VPTex, GLuint &VTex)
{
	if (ins.size() <= 0) {
		RETURN_ON_ERROR("Invalid Ks");
	}
	if (exs.size() != ins.size()) {
		RETURN_ON_ERROR("Invalid W2Cs");
	}

	const size_t nCams = ins.size();
	vector<glm::mat4> _viewMats(nCams);
	vector<glm::mat4> _viewProjMats(nCams);

	/* Transfer cameras' _view matrix */
	for (size_t i = 0; i < nCams; ++i) {
		Extrinsic extrin = exs[i];

		// calculate _view matrix
		glm::mat4 _viewMat = extrin.viewMat;

		// copy to _view matrices buffer
		_viewMats[i] = _viewMat;
	}

	glGenTextures(1, &VTex);
	glBindTexture(GL_TEXTURE_2D, VTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCams * 4, 1, 0, GL_RGBA, GL_FLOAT, _viewMats.data());

	/* Transfer cameras' _view-_proj matrix */
	for (size_t i = 0; i < nCams; ++i) {
		Intrinsic intrin = ins[i];

		// calculate _proj matrix
		glm::mat4 projMat = intrin.ProjMat(near, far, w, h);

		// copy to _view-_proj matrices buffer
		_viewProjMats[i] = projMat * _viewMats[i];
	}

	glGenTextures(1, &VPTex);
	glBindTexture(GL_TEXTURE_2D, VPTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCams * 4, 1, 0, GL_RGBA, GL_FLOAT, _viewProjMats.data());

	return true;
}

Renderer::Renderer(shared_ptr<TereScene> scene)
	: _scene(scene),
	_model(1.f),
	_view(1.f),
	_proj(1.f)
{
	// Assume OpenGL context is valid
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		THROW_ON_ERROR("glew init failed");
	}

	// Compile shaders
	_depthShader = LoadShaders(DEPTH_VS, DEPTH_FS);
	_sceneShader = LoadShaders(SCENE_VS, SCENE_FS);

	// Transmit ref cameras' VP/V
	if (!TransmitCameraToGL(_scene->intrins, _scene->extrins, _scene->width, _scene->height,
		_scene->glnear, _scene->glfar, _VPTexture, _VTexture)) {
		THROW_ON_ERROR("cannot transfer ref cameras' VP/V");
	}

	// Transmit geometry
	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_posBuffer);
	glGenBuffers(1, &_elmBuffer);

	glBindVertexArray(_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, _posBuffer);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX * BYTES_PER_VERTEX, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (scene->dElement) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elmBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_FACE * BYTES_PER_FACE, 0, GL_DYNAMIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

#ifdef USE_CUDA
	CUDA_ERR_CHK(cudaGraphicsGLRegisterBuffer(
		&_cuPosBuffer, _posBuffer, cudaGraphicsMapFlagsWriteDiscard));
	if (scene->dElement) {
		CUDA_ERR_CHK(cudaGraphicsGLRegisterBuffer(
			&_cuElmBuffer, _elmBuffer, cudaGraphicsMapFlagsWriteDiscard));
	}
#endif

	UpdatedGeometry();

	// Transmit textures
	glGenBuffers(1, &_PBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _PBO);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, _scene->width * _scene->height * 3,
		NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	_rgbTextures = vector<GLuint>(_scene->nCams);
	_rgbdTextures = vector<GLuint>(_scene->nCams);
	glGenTextures(_scene->nCams, _rgbTextures.data());
	glGenTextures(_scene->nCams, _rgbdTextures.data());

	for (auto tex : _rgbTextures) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _scene->width, _scene->height);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	for (auto tex : _rgbdTextures) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _scene->width, _scene->height);
#if defined GL_WIN || defined GL_OSX
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
#elif defined GL_ANDROID
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
#elif defined GL_IOS
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

#ifdef USE_CUDA
	CUDA_ERR_CHK(cudaGraphicsGLRegisterBuffer(
		&_cuPBO, _PBO, cudaGraphicsMapFlagsWriteDiscard));
#endif

	UpdatedLF();

	// depth shader uniform locations
	_dVPLct = glGetUniformLocation(_depthShader, "VP");
	_dNearLct = glGetUniformLocation(_depthShader, "near");
	_dFarLct = glGetUniformLocation(_depthShader, "far");

	// scene shader uniform locations
	_sNearLct = glGetUniformLocation(_sceneShader, "near");
	_sFarLct = glGetUniformLocation(_sceneShader, "far");
	_sNCamLct = glGetUniformLocation(_sceneShader, "N_REF_CAMERAS");
	_sVPLct = glGetUniformLocation(_sceneShader, "VP");
	_sVPRefLct = glGetUniformLocation(_sceneShader, "ref_cam_VP");
	_sVRefLct = glGetUniformLocation(_sceneShader, "ref_cam_V");
	_sNInterpLct = glGetUniformLocation(_sceneShader, "nInterps");
	for (int i = 0; i != NUM_INTERP; ++i) {
		string sIndex = string() + "interpIndices[" + TO_STRING(i) + "]";
		string sWeight = string() + "interpWeights[" + TO_STRING(i) + "]";
		string sLf = string() + "lightField[" + TO_STRING(i) + "]";
		_sItpIdLct[i] = glGetUniformLocation(_sceneShader, sIndex.c_str());
		_sItpWtLct[i] = glGetUniformLocation(_sceneShader, sWeight.c_str());
		_sLFLct[i] = glGetUniformLocation(_sceneShader, sLf.c_str());
	}

	// generate frame buffer for scene rendering result
	if (!GenFrameBuffer(_fbo, _cAttach, _dAttach, _scene->width, _scene->height)) {
		THROW_ON_ERROR("Generate scene frame buffer failed\n");
	}

	// generate frame buffer for depth rendering result
	// generated texture is useless for it'll be replaced by RGBD texture in RenderDepth()
	GLuint tempTexture = 0;
	if (!GenFrameBuffer(_rgbdFbo, tempTexture, _rgbdDAttach, _scene->width, _scene->height)) {
		THROW_ON_ERROR("Generate depth frame buffer failed\n");
	}
	glDeleteTextures(1, &tempTexture);
}

bool Renderer::UpdatedGeometry()
{
#ifdef USE_CUDA
	size_t size = 0;
	float* cudaData = nullptr;

	assert(_scene->GPU);

	// Update bound cuda pos buffer
	CUDA_ERR_CHK(cudaGraphicsMapResources(1, &_cuPosBuffer, 0));
	CUDA_ERR_CHK(cudaGraphicsResourceGetMappedPointer((void **)&cudaData,
		&size, _cuPosBuffer));
	assert(_scene->szV < size);
	CUDA_ERR_CHK(cudaMemcpy(cudaData, _scene->v, _scene->szV, cudaMemcpyDeviceToDevice));
	CUDA_ERR_CHK(cudaGraphicsUnmapResources(1, &_cuPosBuffer, 0));

	if (_scene->dElement) {
		// update bound cuda element buffer
		CUDA_ERR_CHK(cudaGraphicsMapResources(1, &_cuElmBuffer, 0));
		CUDA_ERR_CHK(cudaGraphicsResourceGetMappedPointer(
			(void **)&cudaData, &size, _cuElmBuffer));
		assert(_scene->szF < size);
		CUDA_ERR_CHK(cudaMemcpy(cudaData, _scene->f, _scene->szF, cudaMemcpyDeviceToDevice));
		CUDA_ERR_CHK(cudaGraphicsUnmapResources(1, &_cuElmBuffer, 0));
	}
#else
	assert(!_scene->GPU);

	glBindVertexArray(_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, _posBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _scene->szV, _scene->v);

	if (_scene->dElement) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elmBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _scene->szF, _scene->f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#endif /* USE_CUDA */

	_refreshDepth = true;

	return true;
}

bool Renderer::UpdatedLF()
{
#ifdef USE_CUDA
	size_t size = 0;
	float* cudaData = nullptr;

	assert(_scene->GPU);

	for (size_t i = 0; i < _rgbTextures.size(); ++i) {
		CUDA_ERR_CHK(cudaGraphicsMapResources(1, &_cuPBO, 0));
		CUDA_ERR_CHK(cudaGraphicsResourceGetMappedPointer((void **)&cudaData,
			&size, _cuPBO));
		CUDA_ERR_CHK(cudaMemcpy(cudaData, _scene->rgbs[i], 
			_scene->width * _scene->height * 3, cudaMemcpyDeviceToDevice));
		CUDA_ERR_CHK(cudaGraphicsUnmapResources(1, &_cuPBO, 0));

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _PBO);
		glBindTexture(GL_TEXTURE_2D, _rgbTextures[i]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _scene->width, _scene->height,
			GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
#else
	assert(!_scene->GPU);

	for (size_t i = 0; i < _rgbTextures.size(); ++i) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _PBO);
		glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0,
			_scene->width * _scene->height * 3, _scene->rgbs[i]);
		glBindTexture(GL_TEXTURE_2D, _rgbTextures[i]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _scene->width, _scene->height,
			GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
#endif /* USE_CUDA */

	_refreshDepth = true;

	return true;
}

bool Renderer::RefreshDepth()
{
	if (_rgbTextures.size() != _scene->nCams) {
		RETURN_ON_ERROR("_rgbTextures are invalid");
	}
	
	if (_rgbdTextures.size() != _scene->nCams) {
		RETURN_ON_ERROR("_rgbdTextures are invalid");
	}

	for (size_t i = 0; i < _scene->nCams; ++i) {
		GLuint rgb = _rgbTextures[i];
		GLuint rgbd = _rgbdTextures[i];

		// Attach rgbd texture to depth framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, _rgbdFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rgbd, 0);

		// view-proj mat
		Intrinsic intrin(_scene->intrins[i]);
		Extrinsic extrin(_scene->extrins[i]);
		glm::mat4 proj = intrin.ProjMat(_scene->glnear, _scene->glfar, _scene->width, _scene->height);
		glm::mat4 view = extrin.viewMat;
		glm::mat4 vp = proj * view;

		// set up before rendering depth
		glUseProgram(_depthShader);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, _scene->width, _scene->height);
		glUniformMatrix4fv(_dVPLct, 1, GL_FALSE, glm::value_ptr(vp));
		glUniform1f(_dNearLct, _scene->glnear);
		glUniform1f(_dFarLct, _scene->glfar);

		// render depth
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rgb);
		glBindVertexArray(_VAO);
		if (_scene->dElement) {
			glDrawElements(GL_TRIANGLES, _scene->szF / sizeof(int), GL_UNSIGNED_INT, (void*)0);
		}
		else if (_scene->dArray) {
			glDrawArrays(GL_TRIANGLES, 0, _scene->szV / BYTES_PER_VERTEX);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(0);
	}

	_refreshDepth = false;
	return true;
}

//Renderer::Renderer(const LightFieldAttrib &attrib, const size_t fbw, const size_t fbh)
//	: nMeshes(0),
//	indexSizes(),
//	vertexArrays(),
//	vertexBuffers(),
//	elementBuffers(),
//	vColorBuffers(),
//	ref_cam_VP_texture(0),
//	ref_cam_V_texture(0),
//	_sceneShader(0),
//	_depthShader(0),
//	scene_VP_id(0),
//	depth_VP_id(0),
//	depth_near_location(0),
//	depth_far_location(0),
//	scene_near_location(0),
//	scene_far_location(0),
//	N_REF_CAMERAS_location(0),
//	ref_cam_VP_location(0),
//	ref_cam_V_location(0),
//	nInterpsLocation(0),
//	interpIndicesLocation(),
//	interpWeightsLocation(),
//	lightFieldLocation(),
//	_model(1.0f),
//	_view(1.0f),
//	_proj(1.0f),
//	virtual_camera(),
//	_rgbdFrameBuffer(0),
//	_rgbdDepthBuffer(0),
//	_finalFrameBuffer(0),
//	_finalRenderTex(0),
//	_finalDepthBuffer(0),
//	attrib(attrib),
//	interpCameras(),
//	lightFieldTexs(),
//	light_field_H(),
//	useHighTexture(false)
//{
//	// Compile shaders
//	_depthShader = LoadShaders(depth_vertex_code, depth_fragment_code);
//	_sceneShader = LoadShaders(renderer_vertex_code, renderer_fragment_coder);
//
//	// Read objects
//	if (!TransferMeshToGL(attrib.obj_file)) {
//		throw std::runtime_error("cannot read scene");
//	}
//	
//	// Read ref cameras' VP/V
//	if (!TransferRefCameraToGL(attrib.ref_cameras_VP, attrib.ref_cameras_V)) {
//		throw std::runtime_error("cannot transfer ref cameras' VP/V");
//	}
//	
//	// Get uniform locations
//	scene_VP_id = glGetUniformLocation(_sceneShader, "VP");
//	depth_VP_id = glGetUniformLocation(_depthShader, "VP");
//	depth_near_location = glGetUniformLocation(_depthShader, "near");
//	depth_far_location = glGetUniformLocation(_depthShader, "far");
//	scene_near_location = glGetUniformLocation(_sceneShader, "near");
//	scene_far_location = glGetUniformLocation(_sceneShader, "far");
//	N_REF_CAMERAS_location = glGetUniformLocation(_sceneShader, "N_REF_CAMERAS");
//	ref_cam_VP_location = glGetUniformLocation(_sceneShader, "ref_cam_VP");
//	ref_cam_V_location = glGetUniformLocation(_sceneShader, "ref_cam_V");
//	nInterpsLocation = glGetUniformLocation(_sceneShader, "nInterps");
//	for (int i = 0; i != NUM_INTERP; ++i) {
//		string sIndex = string() + "interpIndices[" + TO_STRING(i) + "]";
//		string sWeight = string() + "interpWeights[" + TO_STRING(i) + "]";
//		string sLf = string() + "lightField[" + TO_STRING(i) + "]";
//		interpIndicesLocation[i] = glGetUniformLocation(_sceneShader, sIndex.c_str());
//		interpWeightsLocation[i] = glGetUniformLocation(_sceneShader, sWeight.c_str());
//		lightFieldLocation[i] = glGetUniformLocation(_sceneShader, sLf.c_str());
//	}
//
//	// Initialize MVP matrices
//	_model = glm::mat4(1.0);
//	_view = glm::mat4(1.0);
//	_proj = glm::mat4(1.0);
//
//	// reference textures
//	glGenTextures(NUM_HIGH_INTERP, light_field_H);
//
//	// generate frame buffer for final rendering result
//	if (!GenFrameBuffer(_finalFrameBuffer, _finalRenderTex, _finalDepthBuffer, fbw, fbh)) {
//		throw runtime_error("Generate rendering frame buffer failed\n");
//	}
//}

Renderer::~Renderer()
{
	glDeleteProgram(_sceneShader);
	glDeleteProgram(_depthShader);

	glDeleteTextures(1, &_VPTexture);
	glDeleteTextures(1, &_VTexture);

	glDeleteTextures(_rgbTextures.size(), _rgbTextures.data());

	glDeleteFramebuffers(1, &_fbo);
	glDeleteRenderbuffers(1, &_dAttach);
	glDeleteTextures(1, &_cAttach);

	glDeleteBuffers(1, &_posBuffer);
	glDeleteBuffers(1, &_elmBuffer);
	glDeleteBuffers(1, &_PBO);
	glDeleteVertexArrays(1, &_VAO);
}

int Renderer::Render(const vector<int> &_viewport)
{	
    if (_viewport.size() != 4 || _viewport[0] < 0 || _viewport[1] < 0 ||
        _viewport[2] <= 0 || _viewport[3] <= 0) {
		RETURN_ON_ERROR("invalid viewport");
    }

	if (_refreshDepth) {
		if (!RefreshDepth()) {
			RETURN_ON_ERROR("cannot refresh depth");
		}
		_refreshDepth = false;
	}

	// bind offline framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	
	/* Render scene to screen */
	glUseProgram(_sceneShader);
	glCullFace(GL_BACK);
	EnableMultiSample(false);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// restore _view port
	glViewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);

	// Transfer uniform variables
	glUniform1f(_sNearLct, _scene->glnear);
	glUniform1f(_sFarLct, _scene->glfar);
	glUniform1i(_sNCamLct, _scene->nCams);
	glUniformMatrix4fv(_sVPLct, 1, GL_FALSE, glm::value_ptr(_proj * _view * _model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _VPTexture);
	glUniform1i(_sVPRefLct, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _VTexture);
	glUniform1i(_sVRefLct, 1);
	
    int nInterps = _interpCams.size() < NUM_INTERP ? _interpCams.size() : NUM_INTERP;
	glUniform1i(_sNInterpLct, nInterps);
	for (int i = 0; i != nInterps; ++i) {
		glUniform1i(_sItpIdLct[i], _interpCams[i].index);
		glUniform1f(_sItpWtLct[i], _interpCams[i].weight);
	}

	// Bind light field textures 
	for (int i = 0; i < nInterps; ++i) {
		int camId = _interpCams[i].index;

		if (camId < 0) continue;
		glActiveTexture(GL_TEXTURE3 + i);
		glBindTexture(GL_TEXTURE_2D, _rgbdTextures[camId]);
		glUniform1i(_sLFLct[i], 3 + i);
	}

	// Render scene
	glBindVertexArray(_VAO);
	if (_scene->dElement) {
		glDrawElements(GL_TRIANGLES, _scene->szF / sizeof(int), GL_UNSIGNED_INT, (void*)0);
	}
	else if (_scene->dArray) {
		glDrawArrays(GL_TRIANGLES, 0, _scene->szV / BYTES_PER_VERTEX);
	}
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _cAttach;
}

void Renderer::SetViewer(const glm::mat4 &M, const glm::mat4 &V, const glm::mat4 &P)
{
	_model = M;
	_view = V;
	_proj = P;
}

void Renderer::ClearInterpCameras()
{
	_interpCams.clear();
}

bool Renderer::AddInterpCameras(const WeightedCamera &camera)
{
	_interpCams.push_back(camera);
	return true;
}

//void Renderer::ReplaceHighTexture()
//{
//    // Decode high resolution images
//    string image_file;
//    unsigned char * buf = new unsigned char[attrib.width_H * attrib.height_H * 3];
//    for (int i = 0; i < NUM_HIGH_INTERP; ++i) {
//        int id = interpCameras[i].index;
//        if (id < 0) { continue; }
//        image_file = attrib.image_list[id];
//            
//        // load RGB texture
//        GLuint rgb;
//        Image image = ImageIO::Read(image_file, attrib.width_H, attrib.height_H);
//        std::memcpy(buf, image.GetData(), attrib.width_H * attrib.height_H * 3);
//        glGenTextures(1, &rgb);
//        glBindTexture(GL_TEXTURE_2D, rgb);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, attrib.width_H, attrib.height_H);
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, attrib.width_H, attrib.height_H,
//                        GL_RGB, GL_UNSIGNED_BYTE, buf);
//        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, attrib.width_H, attrib.height_H,
//        //    0, GL_RGB, GL_UNSIGNED_BYTE, buf);
//        glBindTexture(GL_TEXTURE_2D, 0);
//            
//        // render to rgbd texture
//        GLuint rgbd = AppendDepth(rgb, attrib.width_H, attrib.height_H,
//                                    attrib.ref_cameras_VP[id], attrib.ref_cameras_V[id]);
//            
//        // replace original rgbd texture
//        glDeleteTextures(1, &rgb);
//        glDeleteTextures(1, &light_field_H[i]);
//        light_field_H[i] = rgbd;
//    }
//    delete[] buf;
//}

//GLuint Renderer::AppendDepth(GLuint rgb, unsigned int width, unsigned int height,
//	const mat4 &VP, const mat4 &V)
//{
//	if (_rgbdFrameBuffer == 0) {
//		GLuint tempTexture = 0;
//		if (!GenFrameBuffer(_rgbdFrameBuffer, tempTexture, _rgbdDepthBuffer, width, height)) {
//			return 0;
//		}
//		glDeleteTextures(1, &tempTexture);
//		tempTexture = 0;
//	}
//
//	// Generate new texture and attach to framebuffer
//	GLuint rgbd;
//	glGenTextures(1, &rgbd);
//	glBindFramebuffer(GL_FRAMEBUFFER, _rgbdFrameBuffer);
//	
//	glBindTexture(GL_TEXTURE_2D, rgbd);
//	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//#if GL_WIN || GL_OSX
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//#elif GL_ES3_ANDROID
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
//#elif GL_ES3_IOS
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//#endif
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rgbd, 0);
//
//	// render rgbd
//	glUseProgram(_depthShader);
//	glClearColor(0, 0, 0, 1);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glEnable(GL_DEPTH_TEST);
//	glBindVertexArray(_VAO[0]);
//	gl_viewport(0, 0, width, height);
//
//	glUniformMatrix4fv(depth_VP_id, 1, GL_FALSE, glm::value_ptr(VP));
//	glUniform1f(depth_near_location, attrib.glnear);
//	glUniform1f(depth_far_location, attrib.glfar);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, rgb);
//	glDrawElements(GL_TRIANGLES, (int)indexSizes[0], GL_UNSIGNED_INT, (void*)0);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	glBindVertexArray(0);
//
//	return rgbd;
//}

//bool Renderer::TransferMeshToGL(const string &meshFile)
//{
//	if (meshFile.length() < 5) {
//		throw runtime_error(string(" file path too short: ") + meshFile);
//	}
//
//	std::string ext3 = meshFile.substr(meshFile.length() - 3, 3);
//	Geometry geometry;
//
//	if (ext3 == "obj" || ext3 == "OBL") {
//		geometry = Geometry::FromObj(meshFile);
//	}
//	else if (ext3 == "ply" || ext3 == "PLY") {
//		geometry = Geometry::FromPly(meshFile);
//	}
//
//	nMeshes = 1;
//	_VAO = vector<GLuint>(nMeshes, 0);
//	_posBuffer = vector<GLuint>(nMeshes, 0);
//	_elmBuffer = vector<GLuint>(nMeshes, 0);
//	_clrBuffer = vector<GLuint>(nMeshes, 0);
//
//	glGenVertexArrays(nMeshes, _VAO.data());
//	glGenBuffers(nMeshes, _posBuffer.data());
//	glGenBuffers(nMeshes, _elmBuffer.data());
//	glGenBuffers(nMeshes, _clrBuffer.data());
//	indexSizes.clear();
//
//	for (int i = 0; i < nMeshes; ++i)
//	{
//		const float *vertices = geometry.GetVertices().data();
//		int numVertices = geometry.GetVertices().size();
//
//		const int *indices = geometry.GetIndices().data();
//		int numIndex = geometry.GetIndices().size();
//
//		LOGD("   Item %02d: faces: %d\n"
//			 "            vertices: %d\n", i,
//			numIndex / 3, numVertices / 3);
//
//		// Get vertex color
//		int numColors = 0;
//		const uint8_t *colors = nullptr;
//
//		if (geometry.HasColors()) {
//			const vector<uint8_t> &_colors = geometry.GetColors();
//			colors = _colors.data();
//			numColors = _colors.size();
//			LOGI("            colors: %d\n", numColors / 3);
//		}
//		else {
//			LOGI("            colors: 0\n");
//		}
//
//		// Bind VAO
//		glBindVertexArray(_VAO[i]);
//
//		// Bind vertices VBO
//		glBindBuffer(GL_ARRAY_BUFFER, _posBuffer[i]);
//		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), 
//			vertices, GL_STATIC_DRAW);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//
//		if (numColors > 0) {
//			glBindBuffer(GL_ARRAY_BUFFER, _clrBuffer[i]);
//			glBufferData(GL_ARRAY_BUFFER, numColors * sizeof(uint8_t),
//				colors, GL_STATIC_DRAW);
//			glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
//			glEnableVertexAttribArray(1);
//		}
//
//		// Bind EBO
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elmBuffer[i]);
//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndex * sizeof(unsigned int), 
//			indices, GL_STATIC_DRAW);
//
//		glEnableVertexAttribArray(0);
//
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		glBindVertexArray(0);
//
//		indexSizes.push_back(numIndex);
//	}
//
//	return true;
//}

//bool Renderer::TransferRefCameraToGL(const vector<glm::mat4> &refVP, 
//	const vector<glm::mat4> refV)
//{
//	if (refVP.size() != refV.size()) {
//		return false;
//	}
//
//	size_t nCameras = refVP.size();
//
//	// delete buffer if exist
//	glUseProgram(_sceneShader);
//	glDeleteTextures(1, &ref_cam_VP_texture);
//	glDeleteTextures(1, &ref_cam_V_texture);
//
//	/* Transfer cameras' _view-_proj matrix */
//	size_t buffer_size = nCameras * 16;
//	GLfloat *cam_vp_buffer = new GLfloat[buffer_size];
//
//	for (size_t i = 0; i != nCameras; ++i) {
//		glm::mat4 vp = refVP[i];
//		GLfloat *ref_camera_vp = glm::value_ptr(vp);
//		memcpy(cam_vp_buffer + i * 16, ref_camera_vp, 16 * sizeof(GLfloat));
//	}
//
//	glGenTextures(1, &ref_cam_VP_texture);
//	glBindTexture(GL_TEXTURE_2D, ref_cam_VP_texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCameras * 4, 1, 0, GL_RGBA, GL_FLOAT, cam_vp_buffer);
//
//
//	/* Transfer cameras' _view matrix */
//	buffer_size = nCameras * 16;
//	GLfloat *cam_v_buffer = new GLfloat[buffer_size];
//
//	for (size_t i = 0; i != nCameras; ++i) {
//		glm::mat4 v = refV[i];
//		GLfloat *ref_camera_v = glm::value_ptr(v);
//		memcpy(cam_v_buffer + i * 16, ref_camera_v, 16 * sizeof(GLfloat));
//	}
//	glGenTextures(1, &ref_cam_V_texture);
//	glBindTexture(GL_TEXTURE_2D, ref_cam_V_texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCameras * 4, 1, 0, GL_RGBA, GL_FLOAT, cam_v_buffer);
//
//	// release resources
//	delete[] cam_vp_buffer;
//	delete[] cam_v_buffer;
//	glUseProgram(0);
//
//	return true;
//}

//void Renderer::SetLightFieldTexs(const vector<GLuint> &lfTexs)
//{
//	if (lfTexs.size() != attrib.N_REF_CAMERAS) {
//		throw runtime_error("set wrong light field textures");
//	}
//
//	lightFieldTexs = lfTexs;
//}

//void Renderer::GetTextureData(GLuint tex, unsigned char * image)
//{
//#ifdef GL_WIN || GL_OSX
//    glBindTexture(GL_TEXTURE_2D, tex);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//#else 
//#warning "Renderer::GetTextureData not supported with OpenGL ES"
//#endif
//}

//glm::vec3 Renderer::CalcObjCenter(const vector<tinyobj::shape_t>&shapes)
//{
//	glm::vec3 ObjCenter(0.f);
//	int numVertices = 0;
//
//	for (int i = 0; i < shapes.size(); ++i)
//	{
//		int group_numVertices = (int)shapes[i].mesh.positions.size() / 3;
//		numVertices += group_numVertices;
//		for (int j = 0; j < group_numVertices; j++)
//		{
//			ObjCenter.x += shapes[i].mesh.positions[j * 3];
//			ObjCenter.y += shapes[i].mesh.positions[j * 3 + 1];
//			ObjCenter.z += shapes[i].mesh.positions[j * 3 + 2];
//		}
//	}
//
//	ObjCenter.x = ObjCenter.x / numVertices;
//	ObjCenter.y = ObjCenter.y / numVertices;
//	ObjCenter.z = ObjCenter.z / numVertices;
//
//	return (ObjCenter);
//}

