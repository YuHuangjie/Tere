#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <list>
#include "Renderer.h"
#include "common/Shader.hpp"
#include "image/ImageIO.hpp"
#include "mesh/Geometry.hpp"
#include "common/Log.hpp"
#include "shader/renderer_frag.h"
#include "shader/renderer_vs.h"
#include "shader/depth_frag.h"
#include "shader/depth_vs.h"
#include "RayTracer.h"
#include "RenderUtils.h"

using namespace glm;
using namespace std;


Renderer::Renderer(const LightFieldAttrib &attrib, const size_t fbw, const size_t fbh)
	: nMeshes(0),
	indexSizes(),
	vertexArrays(),
	vertexBuffers(),
	elementBuffers(),
	vColorBuffers(),
	ref_cam_VP_texture(0),
	ref_cam_V_texture(0),
	scene_program_id(0),
	depth_program_id(0),
	scene_VP_id(0),
	depth_VP_id(0),
	depth_near_location(0),
	depth_far_location(0),
	scene_near_location(0),
	scene_far_location(0),
	N_REF_CAMERAS_location(0),
	ref_cam_VP_location(0),
	ref_cam_V_location(0),
	nInterpsLocation(0),
	interpIndicesLocation(),
	interpWeightsLocation(),
	lightFieldLocation(),
	Model(1.0f),
	View(1.0f),
	Projection(1.0f),
	virtual_camera(),
	_rgbdFrameBuffer(0),
	_rgbdDepthBuffer(0),
	_finalFrameBuffer(0),
	_finalRenderTex(0),
	_finalDepthBuffer(0),
	attrib(attrib),
	interpCameras(),
	lightFieldTexs(),
	light_field_H(),
	useHighTexture(false)
{
	// Compile shaders
	depth_program_id = LoadShaders(depth_vertex_code, depth_fragment_code);
	scene_program_id = LoadShaders(renderer_vertex_code, renderer_fragment_coder);

	// Read objects
	if (!TransferMeshToGL(attrib.obj_file)) {
		throw std::runtime_error("cannot read scene");
	}
	
	// Read ref cameras' VP/V
	if (!TransferRefCameraToGL(attrib.ref_cameras_VP, attrib.ref_cameras_V)) {
		throw std::runtime_error("cannot transfer ref cameras' VP/V");
	}
	
	// Get uniform locations
	scene_VP_id = glGetUniformLocation(scene_program_id, "VP");
	depth_VP_id = glGetUniformLocation(depth_program_id, "VP");
	depth_near_location = glGetUniformLocation(depth_program_id, "near");
	depth_far_location = glGetUniformLocation(depth_program_id, "far");
	scene_near_location = glGetUniformLocation(scene_program_id, "near");
	scene_far_location = glGetUniformLocation(scene_program_id, "far");
	N_REF_CAMERAS_location = glGetUniformLocation(scene_program_id, "N_REF_CAMERAS");
	ref_cam_VP_location = glGetUniformLocation(scene_program_id, "ref_cam_VP");
	ref_cam_V_location = glGetUniformLocation(scene_program_id, "ref_cam_V");
	nInterpsLocation = glGetUniformLocation(scene_program_id, "nInterps");
	for (int i = 0; i != NUM_INTERP; ++i) {
		string sIndex = string() + "interpIndices[" + TO_STRING(i) + "]";
		string sWeight = string() + "interpWeights[" + TO_STRING(i) + "]";
		string sLf = string() + "lightField[" + TO_STRING(i) + "]";
		interpIndicesLocation[i] = glGetUniformLocation(scene_program_id, sIndex.c_str());
		interpWeightsLocation[i] = glGetUniformLocation(scene_program_id, sWeight.c_str());
		lightFieldLocation[i] = glGetUniformLocation(scene_program_id, sLf.c_str());
	}

	// Initialize MVP matrices
	Model = glm::mat4(1.0);
	View = glm::mat4(1.0);
	Projection = glm::mat4(1.0);

	// reference textures
	glGenTextures(NUM_HIGH_INTERP, light_field_H);

	// generate frame buffer for final rendering result
	if (!GenFrameBuffer(_finalFrameBuffer, _finalRenderTex, _finalDepthBuffer, fbw, fbh)) {
		throw runtime_error("Generate rendering frame buffer failed\n");
	}
}

Renderer::~Renderer()
{
	glDeleteBuffers(nMeshes, vertexBuffers.data());
	glDeleteBuffers(nMeshes, vColorBuffers.data());
	glDeleteBuffers(nMeshes, elementBuffers.data());
	glDeleteVertexArrays(nMeshes, vertexArrays.data());
	glDeleteProgram(scene_program_id);
	glDeleteProgram(depth_program_id);
	glDeleteFramebuffers(1, &_rgbdFrameBuffer);
	glDeleteBuffers(1, &_rgbdDepthBuffer);
	glDeleteTextures(NUM_HIGH_INTERP, light_field_H);
	glDeleteTextures(lightFieldTexs.size(), lightFieldTexs.data());
	glDeleteFramebuffers(1, &_finalFrameBuffer);
	glDeleteTextures(1, &_finalRenderTex);
	glDeleteRenderbuffers(1, &_finalDepthBuffer);
}

int Renderer::Render(const vector<int> &viewport)
{	
	/*
	 * When user interacts (by mouse or finger), render with low resolution
	 * images. When user stops interacting, render with high resolution
	 * images.
	 */
    
    if (viewport.size() != 4 || viewport[0] < 0 || viewport[1] < 0 ||
        viewport[2] <= 0 || viewport[3] <= 0) {
		LOGE("invalid viewport");
		return -1;
    }

	// bind offline framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, _finalFrameBuffer);
	
	/* Render scene to screen */
	glUseProgram(scene_program_id);
	glCullFace(GL_BACK);
	glDisable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// restore view port
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// Transfer uniform variables
	glm::mat4 MVP = Projection * View * Model;
	glUniformMatrix4fv(scene_VP_id, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform1f(scene_near_location, attrib.glnear);
	glUniform1f(scene_far_location, attrib.glfar);
	glUniform1i(N_REF_CAMERAS_location, attrib.N_REF_CAMERAS);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ref_cam_VP_texture);
	glUniform1i(ref_cam_VP_location, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ref_cam_V_texture);
	glUniform1i(ref_cam_V_location, 1);
	
    int nInterps = interpCameras.size() < NUM_INTERP ? interpCameras.size() : NUM_INTERP;
	glUniform1i(nInterpsLocation, nInterps);
	for (int i = 0; i != nInterps; ++i) {
		glUniform1i(interpIndicesLocation[i], interpCameras[i].index);
		glUniform1f(interpWeightsLocation[i], interpCameras[i].weight);
	}

	// Bind light field textures 
	for (int i = 0; i < NUM_HIGH_INTERP && i < nInterps; ++i) {
		int id = interpCameras[i].index;
		if (id < 0) continue;
		glActiveTexture(GL_TEXTURE3 + i);
		if (!useHighTexture) {
			glBindTexture(GL_TEXTURE_2D, lightFieldTexs[id]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, light_field_H[i]);
		}
		glUniform1i(lightFieldLocation[i], 3 + i);
	}

	for (int i = NUM_HIGH_INTERP; i < nInterps; ++i) {
		int id = interpCameras[i].index;
		if (id < 0) continue;
		glActiveTexture(GL_TEXTURE3 + i);
		glBindTexture(GL_TEXTURE_2D, lightFieldTexs[id]);
		glUniform1i(lightFieldLocation[i], 3 + i);
	}

	// Render 2.8D
	for (int i = 0; i < nMeshes; ++i)
	{
		glBindVertexArray(vertexArrays[i]);
		glDrawElements(GL_TRIANGLES, (int)indexSizes[i], GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}

	// release resources
	glUseProgram(0);

	// unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _finalRenderTex;
}

void Renderer::ReplaceHighTexture()
{
    // Decode high resolution images
    string image_file;
    unsigned char * buf = new unsigned char[attrib.width_H * attrib.height_H * 3];
    for (int i = 0; i < NUM_HIGH_INTERP; ++i) {
        int id = interpCameras[i].index;
        if (id < 0) { continue; }
        image_file = attrib.image_list[id];
            
        // load RGB texture
        GLuint rgb;
        Image image = ImageIO::Read(image_file, attrib.width_H, attrib.height_H);
        std::memcpy(buf, image.GetData(), attrib.width_H * attrib.height_H * 3);
        glGenTextures(1, &rgb);
        glBindTexture(GL_TEXTURE_2D, rgb);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, attrib.width_H, attrib.height_H);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, attrib.width_H, attrib.height_H,
                        GL_RGB, GL_UNSIGNED_BYTE, buf);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, attrib.width_H, attrib.height_H,
        //    0, GL_RGB, GL_UNSIGNED_BYTE, buf);
        glBindTexture(GL_TEXTURE_2D, 0);
            
        // render to rgbd texture
        GLuint rgbd = AppendDepth(rgb, attrib.width_H, attrib.height_H,
                                    attrib.ref_cameras_VP[id], attrib.ref_cameras_V[id]);
            
        // replace original rgbd texture
        glDeleteTextures(1, &rgb);
        glDeleteTextures(1, &light_field_H[i]);
        light_field_H[i] = rgbd;
    }
    delete[] buf;
}

GLuint Renderer::AppendDepth(GLuint rgb, unsigned int width, unsigned int height,
	const mat4 &VP, const mat4 &V)
{
	if (_rgbdFrameBuffer == 0) {
		GLuint tempTexture = 0;
		if (!GenFrameBuffer(_rgbdFrameBuffer, tempTexture, _rgbdDepthBuffer, width, height)) {
			return 0;
		}
		glDeleteTextures(1, &tempTexture);
		tempTexture = 0;
	}

	// Generate new texture and attach to framebuffer
	GLuint rgbd;
	glGenTextures(1, &rgbd);
	glBindFramebuffer(GL_FRAMEBUFFER, _rgbdFrameBuffer);
	
	glBindTexture(GL_TEXTURE_2D, rgbd);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#if GL_WIN || GL_OSX
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
#elif GL_ES3_ANDROID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
#elif GL_ES3_IOS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rgbd, 0);

	// render rgbd
	glUseProgram(depth_program_id);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(vertexArrays[0]);
	glViewport(0, 0, width, height);

	glUniformMatrix4fv(depth_VP_id, 1, GL_FALSE, glm::value_ptr(VP));
	glUniform1f(depth_near_location, attrib.glnear);
	glUniform1f(depth_far_location, attrib.glfar);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rgb);
	glDrawElements(GL_TRIANGLES, (int)indexSizes[0], GL_UNSIGNED_INT, (void*)0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);

	return rgbd;
}

bool Renderer::TransferMeshToGL(const string &meshFile)
{
	if (meshFile.length() < 5) {
		throw runtime_error(string(" file path too short: ") + meshFile);
	}

	std::string ext3 = meshFile.substr(meshFile.length() - 3, 3);
	Geometry geometry;

	if (ext3 == "obj" || ext3 == "OBL") {
		geometry = Geometry::FromObj(meshFile);
	}
	else if (ext3 == "ply" || ext3 == "PLY") {
		geometry = Geometry::FromPly(meshFile);
	}

	nMeshes = 1;
	vertexArrays = vector<GLuint>(nMeshes, 0);
	vertexBuffers = vector<GLuint>(nMeshes, 0);
	elementBuffers = vector<GLuint>(nMeshes, 0);
	vColorBuffers = vector<GLuint>(nMeshes, 0);

	glGenVertexArrays(nMeshes, vertexArrays.data());
	glGenBuffers(nMeshes, vertexBuffers.data());
	glGenBuffers(nMeshes, elementBuffers.data());
	glGenBuffers(nMeshes, vColorBuffers.data());
	indexSizes.clear();

	for (int i = 0; i < nMeshes; ++i)
	{
		const float *vertices = geometry.GetVertices().data();
		int numVertices = geometry.GetVertices().size();

		const int *indices = geometry.GetIndices().data();
		int numIndex = geometry.GetIndices().size();

		LOGD("   Item %02d: faces: %d\n"
			 "            vertices: %d\n", i,
			numIndex / 3, numVertices / 3);

		// Get vertex color
		int numColors = 0;
		const uint8_t *colors = nullptr;

		if (geometry.HasColors()) {
			const vector<uint8_t> &_colors = geometry.GetColors();
			colors = _colors.data();
			numColors = _colors.size();
			LOGI("            colors: %d\n", numColors / 3);
		}
		else {
			LOGI("            colors: 0\n");
		}

		// Bind VAO
		glBindVertexArray(vertexArrays[i]);

		// Bind vertices VBO
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), 
			vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		if (numColors > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, vColorBuffers[i]);
			glBufferData(GL_ARRAY_BUFFER, numColors * sizeof(uint8_t),
				colors, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
			glEnableVertexAttribArray(1);
		}

		// Bind EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffers[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndex * sizeof(unsigned int), 
			indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		indexSizes.push_back(numIndex);
	}

	return true;
}

bool Renderer::TransferRefCameraToGL(const vector<glm::mat4> &refVP, 
	const vector<glm::mat4> refV)
{
	if (refVP.size() != refV.size()) {
		return false;
	}

	size_t nCameras = refVP.size();

	// delete buffer if exist
	glUseProgram(scene_program_id);
	glDeleteTextures(1, &ref_cam_VP_texture);
	glDeleteTextures(1, &ref_cam_V_texture);

	/* Transfer cameras' view-projection matrix */
	size_t buffer_size = nCameras * 16;
	GLfloat *cam_vp_buffer = new GLfloat[buffer_size];

	for (size_t i = 0; i != nCameras; ++i) {
		glm::mat4 vp = refVP[i];
		GLfloat *ref_camera_vp = glm::value_ptr(vp);
		memcpy(cam_vp_buffer + i * 16, ref_camera_vp, 16 * sizeof(GLfloat));
	}

	glGenTextures(1, &ref_cam_VP_texture);
	glBindTexture(GL_TEXTURE_2D, ref_cam_VP_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCameras * 4, 1, 0, GL_RGBA, GL_FLOAT, cam_vp_buffer);


	/* Transfer cameras' view matrix */
	buffer_size = nCameras * 16;
	GLfloat *cam_v_buffer = new GLfloat[buffer_size];

	for (size_t i = 0; i != nCameras; ++i) {
		glm::mat4 v = refV[i];
		GLfloat *ref_camera_v = glm::value_ptr(v);
		memcpy(cam_v_buffer + i * 16, ref_camera_v, 16 * sizeof(GLfloat));
	}
	glGenTextures(1, &ref_cam_V_texture);
	glBindTexture(GL_TEXTURE_2D, ref_cam_V_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nCameras * 4, 1, 0, GL_RGBA, GL_FLOAT, cam_v_buffer);

	// release resources
	delete[] cam_vp_buffer;
	delete[] cam_v_buffer;
	glUseProgram(0);

	return true;
}

void Renderer::SetLightFieldTexs(const vector<GLuint> &lfTexs)
{
	if (lfTexs.size() != attrib.N_REF_CAMERAS) {
		throw runtime_error("set wrong light field textures");
	}

	lightFieldTexs = lfTexs;
}

void Renderer::SetVirtualCamera(const Camera &camera)
{
	virtual_camera = camera;
	View = camera.GetViewMatrix();
	Projection = camera.GetProjectionMatrix(attrib.glnear, attrib.glfar);
	//SearchInterpCameras();
}

void Renderer::ClearInterpCameras()
{
	interpCameras.clear();
}

bool Renderer::AddInterpCameras(const WeightedCamera &camera)
{
	interpCameras.push_back(camera);
	return true;
}

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

