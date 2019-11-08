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
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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
