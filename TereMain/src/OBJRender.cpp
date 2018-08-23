#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <list>
#include "OBJRender.h"
#include "common/Shader.hpp"
#include "image/ImageIO.hpp"
#include "mesh/Geometry.hpp"
#include "common/Log.hpp"
#include "shader/renderer_frag.h"
#include "shader/renderer_vs.h"
#include "shader/depth_frag.h"
#include "shader/depth_vs.h"
#include "RayTracer.h"

using namespace glm;
using namespace std;


OBJRender::OBJRender(const LightFieldAttrib &attrib, int fbWidth, int fbHeight)
	: nMeshes(0),
	indexSizes(),
	scene_center(0.f),
	vertexArrays(),
	vertexBuffers(),
	elementBuffers(),
	vColorBuffers(),
	attrib(attrib),
	interpCameras()
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

	// Load camera mesh
	if (!LoadCameraMesh(attrib.camera_mesh_name)) {
		// throw std::runtime_error("cannot read camera mesh");
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

	// Create frame buffer for offline rendering
	glGenFramebuffers(1, &frameBuffer);
	glGenTextures(1, &renderedTexture);
	glGenRenderbuffers(1, &depthBuffer);

	// bind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	
	// bind color texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbWidth, fbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	// the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
#if GL_WIN || GL_OSX
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbWidth, fbHeight);
#elif GL_ES3_IOS || GL_ES3_ANDROID
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbWidth, fbHeight);
#endif
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	
	// check if the framebuffer is actually complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw runtime_error("Framebuffer is not complete!\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(NUM_HIGH_INTERP, light_field_H);

	useHighTexture = false; 
}

OBJRender::~OBJRender()
{
	glDeleteBuffers(nMeshes, vertexBuffers.data());
	glDeleteBuffers(nMeshes, vColorBuffers.data());
	glDeleteBuffers(nMeshes, elementBuffers.data());
	glDeleteVertexArrays(nMeshes, vertexArrays.data());
	glDeleteProgram(scene_program_id);
	glDeleteProgram(depth_program_id);
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &renderedTexture);
	glDeleteBuffers(1, &depthBuffer);
	glDeleteTextures(NUM_HIGH_INTERP, light_field_H);
	glDeleteTextures(lightFieldTexs.size(), lightFieldTexs.data());
}

int OBJRender::render(const vector<int> &viewport)
{	
	/*
	 * When user interacts (by mouse or finger), render with low resolution
	 * images. When user stops interacting, render with high resolution
	 * images.
	 */
    
    if (viewport.size() != 4 || viewport[0] < 0 || viewport[1] < 0 ||
        viewport[2] <= 0 || viewport[3] <= 0) {
        throw std::runtime_error("invalid viewport");
    }
	
	/* Render scene to screen */
	glUseProgram(scene_program_id);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
    
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

	return 0;
}

void OBJRender::ReplaceHighTexture()
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

GLuint OBJRender::AppendDepth(GLuint rgb, unsigned int width, unsigned int height,
	const mat4 &VP, const mat4 &V)
{
	// Generate new texture and attach to framebuffer
	GLuint rgbd;
	glGenTextures(1, &rgbd);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	
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

	return rgbd;
}

bool OBJRender::TransferMeshToGL(const string &meshFile)
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
		vector<float> fcolors;
		int numColors = 0;
		if (geometry.HasColors()) {
			const vector<uint8_t> &colors = geometry.GetColors();
			fcolors = vector<float>(colors.begin(), colors.end());
			std::for_each(fcolors.begin(), fcolors.end(), 
				[](float &c) { c = c / 255.f; });
			numColors = fcolors.size();
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
			glBufferData(GL_ARRAY_BUFFER, numColors * sizeof(float),
				fcolors.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
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

bool OBJRender::TransferRefCameraToGL(const vector<glm::mat4> &refVP, 
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

bool OBJRender::LoadCameraMesh(const string &cameraMeshName)
{
	ifstream camera_mesh_file(cameraMeshName);

	if (!camera_mesh_file.is_open()) { 
		return false;
	}

	string line;
	istringstream ss_line;
	float p1, p2, p3;
	int id1 = -1;
	int id2 = -1;
	int id3 = -1;
	int id4 = 0;

	while (!camera_mesh_file.eof()) {
		getline(camera_mesh_file, line);

		if (line.empty() || line[0] == '#') continue;
		else if (line[0] == 'v') {
			ss_line.str(line.substr(2));
			ss_line >> p1 >> p2 >> p3;
			camera_vertices.push_back(glm::vec3(p1, p2, p3));
			ss_line.clear();
		}
		else if (line[0] == 'f') {
			id1 = -1; id2 = -1; id3 = -1; id4 = 0;
			ss_line.str(line.substr(2));
			ss_line >> id1 >> id2 >> id3 >> id4;
			// .obj is 1 based
			camera_quads.push_back(glm::ivec4(id1 - 1, id2 - 1, id3 - 1, id4 - 1));
			ss_line.clear();
		}
		else {
			LOGW("unrecognized obj content %s\n", line.c_str());
		}
	}

	return true;
}

void OBJRender::SearchInterpCameras(void)
{
	// Prepare directions from reference camera to scene_center
	if (ref_camera_dirs.empty()) {
		for (vector<Camera>::const_iterator it = attrib.ref_cameras.cbegin();
			it != attrib.ref_cameras.cend(); ++it)
		{
			ref_camera_dirs.push_back(glm::normalize(it->GetPosition() - 
				attrib.ref_camera_center));
			ref_camera_dists.push_back(2.0f);
		}
		ref_camera_index = vector<int>(ref_camera_dirs.size());
		std::iota(ref_camera_index.begin(), ref_camera_index.end(), 0);
	}

	// Ray-quad intersection
	bool intersected = false;
	int intersected_quad_id = -1;
	glm::vec3 origin = attrib.ref_camera_center;
	glm::vec3 view_vec = virtual_camera.GetDir();

	intersected_quad_id = RayQuadIntersect(camera_vertices, camera_quads,
		origin, view_vec);

	if (intersected_quad_id >= 0) {
		intersected = true;
	}

	interpCameras.clear();

	// Weight function: W = exp(-(x-1)^2 / (2/9))
	if (intersected) {
		// directly intersected quad
		for (int i = 0; i != 4; ++i) {
			int index = camera_quads[intersected_quad_id][i];
			if (index < 0) {
				continue;
			}

			float cosDist = glm::dot(view_vec, attrib.ref_cameras[index].GetDir());
			cosDist = std::pow(cosDist, 4);
			float weight = std::exp(-(cosDist-1)*(cosDist-1)/0.2222f);  
			interpCameras.push_back(WeightedCamera(index, weight));
		}

		// Look for indirect cameras
		// calculate cosine distance of ref_camera_dir with view_vec
		for (int i = 0; i != ref_camera_dirs.size() && i != ref_camera_dists.size(); ++i) {
			ref_camera_dists[i] = glm::dot(ref_camera_dirs[i], view_vec);
		}

		// get sorted index (starting from least distance)
		const size_t how_many_sort = 12;

		std::partial_sort(ref_camera_index.begin(), ref_camera_index.begin() + how_many_sort,
			ref_camera_index.end(), [this](int a, int b) {
			return ref_camera_dists[a] > ref_camera_dists[b];
		});
		//std::sort(ref_camera_index.begin(), ref_camera_index.end(), [this](int a, int b) {
		//	return ref_camera_dists[a] > ref_camera_dists[b];
		//});

		// extract first twelve indices
		std::list<int> first_twelve(ref_camera_index.begin(), ref_camera_index.begin() + how_many_sort);
		// and remvoe already interpolated from them
		first_twelve.remove_if([this](int a) {
			bool flag = (a == interpCameras[0].index || a == interpCameras[1].index ||
				a == interpCameras[2].index);
			flag |= (interpCameras.size() > 3 ? a==interpCameras[3].index : false);
			return flag;
		});

		// eight indices
		std::list<int>::const_iterator itEight = first_twelve.cbegin();

		for (int i = 0; i != how_many_sort - 4; ++i) {
			int index = *itEight++;
			float cosDist = glm::dot(view_vec, attrib.ref_cameras[index].GetDir());
			cosDist = std::pow(cosDist, 4);
			float weight = std::exp(-(cosDist - 1)*(cosDist - 1) / 0.2222f);
			interpCameras.push_back(WeightedCamera(index, weight));
		}
	}
}

void OBJRender::SetLightFieldTexs(const vector<GLuint> &lfTexs)
{
	if (lfTexs.size() != attrib.N_REF_CAMERAS) {
		throw runtime_error("set wrong light field textures");
	}

	lightFieldTexs = lfTexs;
}

void OBJRender::SetVirtualCamera(const Camera &camera)
{
	virtual_camera = camera;
	View = camera.GetViewMatrix();
	Projection = camera.GetProjectionMatrix(attrib.glnear, attrib.glfar);
	//SearchInterpCameras();
}

void OBJRender::ClearInterpCameras()
{
	interpCameras.clear();
}

bool OBJRender::AddInterpCameras(const WeightedCamera &camera)
{
	interpCameras.push_back(camera);
	return true;
}

//void OBJRender::GetTextureData(GLuint tex, unsigned char * image)
//{
//#ifdef GL_WIN || GL_OSX
//    glBindTexture(GL_TEXTURE_2D, tex);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//#else 
//#warning "OBJRender::GetTextureData not supported with OpenGL ES"
//#endif
//}

//glm::vec3 OBJRender::CalcObjCenter(const vector<tinyobj::shape_t>&shapes)
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

GLuint OBJRender::LoadShaders(const char * vertex_code, const char * fragment_code)
{
	// Compile shaders
	Shader shader(vertex_code, fragment_code);
	return shader.ID;
}

