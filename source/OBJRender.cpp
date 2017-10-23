/* References for understanding OpenGL
* - http://www.learnopengl.com/ (Really good)
* - https://open.gl/ (provides source code)
* - http://www.opengl-tutorial.org/ (some bits don't work or just aren't explained well)
* - http://mbsoftworks.sk/index.php?page=tutorials&series=1 (Haven't looked at it much, but has some cool topics)
* - https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/index.php (to understand shaders and GLSL, but some of the syntax is out of date)
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <list>
#include "OBJRender.h"
#include "Decoder.h"
#include "Shader.h"
#include "renderer_frag.h"
#include "renderer_vs.h"
#include "depth_frag.h"
#include "depth_vs.h"

using namespace glm;
using namespace std;

OBJRender::OBJRender(std::string obj_name, int w, int h, 
	string camera_mesh_name)
	: camera_set(CLOSEST_CAMERA_NR), ind_camera_set(4)
	, dind_camera_set(4)
{
	targetWidth = w;
	targetHeight = h;

	// load obj model
	std::string error;
	if (!tinyobj::LoadObj(shapes, materials, error, obj_name.c_str())) {
		throw runtime_error(error);
	}
	scene_center = CalcObjCenter();

	/* Read model into OpenGL */
	texelbuffers = VertexArray = vertexbuffers = elementbuffers = normalbuffers = NULL;
	TransferMeshToGL();

	/* Compile shaders */
	depth_program_id = LoadShaders(depth_vertex_code, depth_fragment_code);
	scene_program_id = LoadShaders(renderer_vertex_code, renderer_fragment_coder);

	/* Get shader uniform variables for future change */
	scene_VP_id = glGetUniformLocation(scene_program_id, "VP");
	depth_VP_id = glGetUniformLocation(depth_program_id, "VP");
	depth_V_id = glGetUniformLocation(depth_program_id, "V");
	depth_near_location = glGetUniformLocation(depth_program_id, "near");
	depth_far_location = glGetUniformLocation(depth_program_id, "far");
	scene_near_location = glGetUniformLocation(scene_program_id, "near");
	scene_far_location = glGetUniformLocation(scene_program_id, "far");
	N_REF_CAMERAS_HIGH_location = glGetUniformLocation(scene_program_id, "N_REF_CAMERAS_HIGH");
	N_REF_CAMERAS_LOW_location = glGetUniformLocation(scene_program_id, "N_REF_CAMERAS_LOW");
	N_REF_CAMERAS_location = glGetUniformLocation(scene_program_id, "N_REF_CAMERAS");
	WEIGHTS_location = glGetUniformLocation(scene_program_id, "WEIGHTS");
	INDIRECT_WEIGHTS_location = glGetUniformLocation(scene_program_id, "INDIRECT_WEIGHTS");
	DINDIRECT_WEIGHTS_location = glGetUniformLocation(scene_program_id, "DINDIRECT_WEIGHTS");
	CAMERAS_location = glGetUniformLocation(scene_program_id, "CAMERAS");
	INDIRECT_CAMERAS_location = glGetUniformLocation(scene_program_id, "INDIRECT_CAMERAS");
	DINDIRECT_CAMERAS_location = glGetUniformLocation(scene_program_id, "DINDIRECT_CAMERAS");
	ref_cam_VP_location = glGetUniformLocation(scene_program_id, "ref_cam_VP");
	ref_cam_V_location = glGetUniformLocation(scene_program_id, "ref_cam_V");
	for (int i = 0; i != CLOSEST_CAMERA_NR; ++i) {
		string lf = string() + "light_field[" + TO_STRING(i) + "]";
		string T_lf = string() + "T_light_field[" + TO_STRING(i) + "]";
		string T2_lf = string() + "T2_light_field[" + TO_STRING(i) + "]";
		light_field_location[i] = glGetUniformLocation(scene_program_id, lf.c_str());
		T_light_field_location[i] = glGetUniformLocation(scene_program_id, T_lf.c_str());
		T2_light_field_location[i] = glGetUniformLocation(scene_program_id, T2_lf.c_str());
	}

	Model = glm::mat4(1.0);
	View = glm::mat4(1.0);
	Projection = glm::mat4(1.0);
	render_camera_location = glm::vec3();

	/* Render to frame buffer */
	/* Doc: https://learnopengl.com/#!Advanced-OpenGL/Framebuffers */
	glGenFramebuffers(1, FramebufferName);
	glGenTextures(1, renderedTexture);
	glGenRenderbuffers(1, depthrenderbuffer);

	for (int i = 0; i < 1; ++i)
	{
		// Bind frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[i]);

		// Bind color texture
		glBindTexture(GL_TEXTURE_2D, renderedTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, targetWidth, targetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		// Attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[i], 0);

		// The depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer[i]);
#ifdef GL
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, targetWidth, targetHeight);
#elif defined GL_ES3
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, targetWidth, targetHeight);
#endif
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer[i]);

		// Check if the framebuffer is actually complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw runtime_error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/* load camera mesh */
	LoadCameraMesh(camera_mesh_name);

	glGenTextures(CLOSEST_CAMERA_NR, light_field_H);

	interacting = false; 
}

OBJRender::~OBJRender()
{
	glDeleteBuffers((int)shapes.size(), vertexbuffers);
	glDeleteBuffers((int)shapes.size(), elementbuffers);
	glDeleteBuffers((int)shapes.size(), normalbuffers);
	glDeleteBuffers((int)shapes.size(), texelbuffers);
	glDeleteVertexArrays((int)shapes.size(), VertexArray);
	glDeleteProgram(scene_program_id);
	glDeleteProgram(depth_program_id);
	glDeleteFramebuffers(1, FramebufferName);
	glDeleteTextures(1, renderedTexture);
	glDeleteBuffers(1, depthrenderbuffer);
	glDeleteBuffers(1, &lookup_table_buffer);
	glDeleteBuffers(1, &ref_cam_MVP_buffer);
	glDeleteBuffers(1, &ref_cam_MV_buffer);
	glDeleteTextures(CLOSEST_CAMERA_NR, light_field_H);
}

int OBJRender::render()
{	
	/* Render depth maps to texture */
	//glUseProgram(depth_program_id);
	//glClearColor(0, 0, 0, 1);
	//glEnable(GL_DEPTH_TEST);
	//glBindVertexArray(VertexArray[0]);
	//glViewport(0, 0, targetWidth, targetHeight);

	//for (int i = 0; i != 4; ++i) {
	//	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[i]);

	//	int camera_id = camera_set.GetCamera(i);
	//	if (camera_id == -1 || !camera_set.IsModified(i)) continue;
	//	
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	glUniformMatrix4fv(depth_VP_id, 1, GL_FALSE, glm::value_ptr(attrib.ref_cameras_VP[camera_id]));
	//	glUniformMatrix4fv(depth_V_id, 1, GL_FALSE,	glm::value_ptr(attrib.ref_cameras_V[camera_id]));
	//	glUniform1f(depth_near_location, attrib.near);
	//	glUniform1f(depth_far_location, attrib.far);

	//	glDrawElements(GL_TRIANGLES, (int)shapes[0].mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
	//}

	/*
	* Here's the idea: When user interacts (by mouse or finger), render by low
	* resolution images. When user stops interacting, render by high resolution
	* images.
	*/

	bool use_high_res = !interacting;

	if (use_high_res) {
		string image_file;
		unsigned char * buf = new unsigned char[attrib.width_H * attrib.height_H * 3];
		for (int i = 0; i != CLOSEST_CAMERA_NR; ++i) {
			int id = camera_set.GetCamera(i);
			if (id < 0) { continue; }
			image_file = attrib.image_file_prefix + attrib.image_list[id];

			// load RGB texture
			GLuint rgb;
			::Decompress(image_file, buf, attrib.width_H, attrib.height_H);
			glGenTextures(1, &rgb);
			glBindTexture(GL_TEXTURE_2D, rgb);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, attrib.width_H, attrib.height_H,
				0, GL_RGB, GL_UNSIGNED_BYTE, buf);
			glBindTexture(GL_TEXTURE_2D, 0);

			// render to rgbd texture
			GLuint rgbd = RenderRGBD(id, rgb, attrib.width_H, attrib.height_H);

			// replace original rgbd texture
			glDeleteTextures(1, &rgb);
			glDeleteTextures(1, &light_field_H[i]);
			light_field_H[i] = rgbd;
		}
		delete[] buf;
	}

	/* Render scene to screen */
	glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
	glUseProgram(scene_program_id);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// fit image into screen
	float aspect = glm::max((float)attrib.width_H / targetWidth, (float)attrib.height_H / targetHeight);
	glViewport(targetWidth / 2 - attrib.width_H / aspect / 2, targetHeight / 2 - attrib.height_H / aspect / 2,
		attrib.width_H / aspect, attrib.height_H / aspect);

	/* Transfer uniform variables */
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
	
	glUniformMatrix4fv(scene_VP_id, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform1i(N_REF_CAMERAS_HIGH_location, attrib.N_REF_CAMERAS_HIGH);
	glUniform1i(N_REF_CAMERAS_LOW_location, attrib.N_REF_CAMERAS_LOW);
	glUniform1i(N_REF_CAMERAS_location, attrib.N_REF_CAMERAS);
	glUniform4fv(WEIGHTS_location, 1, glm::value_ptr(camera_set.GetWeight()));
	glUniform4fv(INDIRECT_WEIGHTS_location, 1, glm::value_ptr(ind_camera_set.GetWeight()));
	glUniform4fv(DINDIRECT_WEIGHTS_location, 1, glm::value_ptr(dind_camera_set.GetWeight()));
	glUniform4iv(CAMERAS_location, 1, glm::value_ptr(camera_set.GetCamera()));
	glUniform4iv(INDIRECT_CAMERAS_location, 1, glm::value_ptr(ind_camera_set.GetCamera()));
	glUniform4iv(DINDIRECT_CAMERAS_location, 1, glm::value_ptr(dind_camera_set.GetCamera()));
	glUniform1f(scene_near_location, attrib.glnear);
	glUniform1f(scene_far_location, attrib.glfar);

	/* Bind textures */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ref_cam_VP_texture);
	glUniform1i(ref_cam_VP_location, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ref_cam_V_texture);
	glUniform1i(ref_cam_V_location, 1);
	for (int i = 0; i != CLOSEST_CAMERA_NR; ++i) {
		int id = camera_set.GetCamera(i);
		if (id < 0) continue;
		glActiveTexture(GL_TEXTURE3 + i);
		if (!use_high_res) {
			glBindTexture(GL_TEXTURE_2D, attrib.light_field_tex[id]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, light_field_H[i]);
		}
		glUniform1i(light_field_location[i], 3 + i);
	}
	for (int i = 0; i != CLOSEST_CAMERA_NR; ++i) {
		int id = ind_camera_set.GetCamera(i);
		if (id < 0) continue;
		glActiveTexture(GL_TEXTURE3 + 1*CLOSEST_CAMERA_NR + i);
		glBindTexture(GL_TEXTURE_2D, attrib.light_field_tex[id]);
		glUniform1i(T_light_field_location[i], 3 + 1*CLOSEST_CAMERA_NR + i);
	}
	for (int i = 0; i != CLOSEST_CAMERA_NR; ++i) {
		int id = dind_camera_set.GetCamera(i);
		if (id < 0) continue;
		glActiveTexture(GL_TEXTURE3 + 2*CLOSEST_CAMERA_NR + i);
		glBindTexture(GL_TEXTURE_2D, attrib.light_field_tex[id]);
		glUniform1i(T2_light_field_location[i], 3 + 2*CLOSEST_CAMERA_NR + i);
	}

	// output rendered object
	for (int i = 0; i < (int)shapes.size(); ++i)
	{
		glBindVertexArray(VertexArray[i]);
		glDrawElements(GL_TRIANGLES, (int)shapes[i].mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}

	/* release resources */
	glUseProgram(0);

	return 0;
}

GLuint OBJRender::RenderRGBD(int id, GLuint rgb, unsigned int width, unsigned int height)
{
	// Generate new texture and attach to framebuffer
	GLuint rgbd;
	glGenTextures(1, &rgbd);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[0]);
	
	glBindTexture(GL_TEXTURE_2D, rgbd);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#ifdef GL
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
#elif defined GL_ES3 && defined __ANDROID__
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
#elif defined GL_ES3 && defined IOS
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
	glBindVertexArray(VertexArray[0]);
	glViewport(0, 0, width, height);

	glUniformMatrix4fv(depth_VP_id, 1, GL_FALSE, glm::value_ptr(attrib.ref_cameras_VP[id]));
	glUniformMatrix4fv(depth_V_id, 1, GL_FALSE, glm::value_ptr(attrib.ref_cameras_V[id]));
	glUniform1f(depth_near_location, attrib.glnear);
	glUniform1f(depth_far_location, attrib.glfar);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rgb);
	glDrawElements(GL_TRIANGLES, (int)shapes[0].mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return rgbd;
}

void OBJRender::TransferLookupTableToGL(const vector<int> &table)
{
    //look up table is going to be stored in SSBO, maximum memory allowed is 2GB
#ifdef GL
    glGenBuffers(1, &lookup_table_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lookup_table_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, table.size() * sizeof(int),
        table.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lookup_table_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    lookup_table = table;
#else
#warning "OBJRender::TransferLookupTableToGL not supported with OpenGL ES"
#endif
}

void OBJRender::TransferMeshToGL(void)
{
	int n = (int)shapes.size();
	VertexArray = new GLuint[n];
	normalbuffers = new GLuint[n];
	vertexbuffers = new GLuint[n];
	texelbuffers = new GLuint[n];
	elementbuffers = new GLuint[n];

	glGenVertexArrays(n, VertexArray);
	glGenBuffers(n, vertexbuffers);
	glGenBuffers(n, elementbuffers);
	glGenBuffers(n, normalbuffers);
	glGenBuffers(n, texelbuffers);

	for (int i = 0; i < n; ++i)
	{
		float *vertices = shapes[i].mesh.positions.data();
		int numVertices = (int)shapes[i].mesh.positions.size();

		unsigned int *indices = shapes[i].mesh.indices.data();
		int numIndex = (int)shapes[i].mesh.indices.size();

		//float *texcoord = shapes[i].mesh.texcoords.data();
		//int numTexCoord = (int)shapes[i].mesh.texcoords.size();

		//float *Norms = shapes[i].mesh.normals.data();
		//int numNormals = (int)shapes[i].mesh.normals.size();

		LOGD("Item %d: %d faces, %d vertices\n", i, numIndex / 3, numVertices / 3);

		// Bind VAO
		glBindVertexArray(VertexArray[i]);

		// Bind vertices VBO
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//// Bind texture coordinates VBO
		//glBindBuffer(GL_ARRAY_BUFFER, texelbuffers[i]);
		//glBufferData(GL_ARRAY_BUFFER, numTexCoord * sizeof(float), texcoord, GL_STATIC_DRAW);
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//// Bind normals VBO
		//glBindBuffer(GL_ARRAY_BUFFER, normalbuffers[i]);
		//glBufferData(GL_ARRAY_BUFFER, numNormals * sizeof(float), Norms, GL_STATIC_DRAW);
		//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Bind EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffers[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndex * sizeof(unsigned int), indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		//glEnableVertexAttribArray(1);
		//glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void OBJRender::TransferRefCameraToGL(void)
{
	// delete buffer if exist
	glUseProgram(scene_program_id);
	glDeleteTextures(1, &ref_cam_VP_texture);
	glDeleteTextures(1, &ref_cam_V_texture);

	/* Transfer cameras' view-projection matrix */
	size_t buffer_size = attrib.N_REF_CAMERAS * 16;
	GLfloat *cam_vp_buffer = new GLfloat[buffer_size];

	for (size_t i = 0; i != attrib.ref_cameras.size(); ++i) {
		glm::mat4 vp = attrib.ref_cameras_VP[i];
		GLfloat *ref_camera_vp = glm::value_ptr(vp);
		memcpy(cam_vp_buffer + i * 16, ref_camera_vp, 16 * sizeof(GLfloat));
	}

	glGenTextures(1, &ref_cam_VP_texture);
	glBindTexture(GL_TEXTURE_2D, ref_cam_VP_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, attrib.N_REF_CAMERAS * 4 + 1, 1, 0, GL_RGBA, GL_FLOAT, cam_vp_buffer);


	/* Transfer cameras' view matrix */
	buffer_size = attrib.N_REF_CAMERAS * 16;
	GLfloat *cam_v_buffer = new GLfloat[buffer_size];

	for (size_t i = 0; i != attrib.ref_cameras.size(); ++i) {
		glm::mat4 v = attrib.ref_cameras_V[i];
		GLfloat *ref_camera_v = glm::value_ptr(v);
		memcpy(cam_v_buffer + i * 16, ref_camera_v, 16 * sizeof(GLfloat));
	}
	glGenTextures(1, &ref_cam_V_texture);
	glBindTexture(GL_TEXTURE_2D, ref_cam_V_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, attrib.N_REF_CAMERAS * 4 + 1, 1, 0, GL_RGBA, GL_FLOAT, cam_v_buffer);

	// release resources
	delete[] cam_vp_buffer;
	delete[] cam_v_buffer;
	glUseProgram(0);
}

void OBJRender::LoadCameraMesh(const string &camera_mesh_name)
{
	ifstream camera_mesh_file(camera_mesh_name);

	if (!camera_mesh_file.is_open()) { throw runtime_error("cannot open camera mesh"); }

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
}

void OBJRender::LocateClosestCameras(void)
{
	/* Prepare directions from reference camera to scene_center */
	if (ref_camera_dirs.empty()) {
		for (vector<RenderCamView>::const_iterator it = attrib.ref_cameras.cbegin();
			it != attrib.ref_cameras.cend(); ++it)
		{
			ref_camera_dirs.push_back(glm::normalize(it->GetPosition() - scene_center));
			ref_camera_dists.push_back(2.0f);
		}
		ref_camera_index = vector<int>(ref_camera_dirs.size());
		std::iota(ref_camera_index.begin(), ref_camera_index.end(), 0);
	}

	/* ray-quad intersection test */
	const float EPS = 1e-5;
	bool intersected = false;
	int intersected_quad_id = -1;
	glm::vec3 origin = attrib.ref_camera_center;
	//glm::vec3 origin = scene_center;
	//glm::vec3 view_vec = glm::normalize(render_camera_location - origin);
	glm::vec3 view_vec = -virtual_camera.GetLookAt();

	for (vector<glm::ivec4>::iterator it = camera_quads.begin(); 
		it != camera_quads.end() && !intersected; ++it) {
		glm::ivec4 quad_indices = *it;

		for (int i = 0; i != 2; ++i) {
			glm::vec3 v1, v2, v3;
			if (i == 0) {
				v1 = camera_vertices[quad_indices.x];
				v2 = camera_vertices[quad_indices.y];
				v3 = camera_vertices[quad_indices.z];
			}
			else if (quad_indices.w != -1) {
				v1 = camera_vertices[quad_indices.x];
				v2 = camera_vertices[quad_indices.z];
				v3 = camera_vertices[quad_indices.w];
			}
			else {
				break;
			}

			glm::vec3 u = v2 - v1;
			glm::vec3 v = v3 - v1;
			glm::vec3 normal = glm::normalize(glm::cross(u, v));

			// ray-plane parallel test
			glm::vec3 w0 = attrib.ref_camera_center - v1;
			float a = -glm::dot(normal, w0);
			float b = glm::dot(normal, view_vec);

			if (abs(b) < EPS) { continue; }

			// get intersection point of ray with triangle plane
			float r = a / b;
			if (r < 0.0f) { continue; }		// ray goes away from triangle plane

			// point inside triangle test
			glm::vec3 p = origin + view_vec * r;
			float uu, uv, vv, wu, wv, D;
			glm::vec3 w;
			uu = glm::dot(u, u);
			uv = glm::dot(u, v);
			vv = glm::dot(v, v);
			w = p - v1;
			wu = glm::dot(w, u);
			wv = glm::dot(w, v);
			D = uv * uv - uu * vv;

			// get and test parametric coords
			float s, t;
			s = (uv * wv - vv * wu) / D;
			if (s < 0.0f || s > 1.0f) { continue; }
			t = (uv * wu - uu * wv) / D;
			if (t < 0.0f || (s + t) > 1.0f) { continue; }


			intersected = true;
			intersected_quad_id = (int)(it - camera_quads.begin());
			break;
		}
	}

	if (intersected) {
		// Found direct cameras
		int camera_a = camera_quads[intersected_quad_id][0];
		int camera_b = camera_quads[intersected_quad_id][1];
		int camera_c = camera_quads[intersected_quad_id][2];
		int camera_d = camera_quads[intersected_quad_id][3];

		float weight_a = 1.0 / (1.0001 - glm::dot(view_vec, attrib.ref_cameras[camera_a].GetLookAt()));
		float weight_b = 1.0 / (1.0001 - glm::dot(view_vec, attrib.ref_cameras[camera_b].GetLookAt()));
		float weight_c = 1.0 / (1.0001 - glm::dot(view_vec, attrib.ref_cameras[camera_c].GetLookAt()));
		float weight_d = (camera_d == -1 ? 0.0f : 
			1.0 / (1.0001 - glm::dot(view_vec, attrib.ref_cameras[camera_d].GetLookAt())));

		camera_set.SetCameras(glm::ivec4(camera_a, camera_b, camera_c, camera_d),
			glm::vec4(weight_a, weight_b, weight_c, weight_d));

		/* Look for indirect cameras */
	
		// Calculate cosine distance of ref_camera_dir with view_vec
		for (int i = 0; i != ref_camera_dirs.size() && i != ref_camera_dists.size(); ++i) {
			ref_camera_dists[i] = glm::dot(ref_camera_dirs[i], view_vec);
		}
		// Get sorted index (starting from least distance)
		std::sort(ref_camera_index.begin(), ref_camera_index.end(), [this](int a, int b) {
			return ref_camera_dists[a] > ref_camera_dists[b];
		});
		// Extract first twelve indices
		std::list<int> first_twelve(ref_camera_index.begin(), ref_camera_index.begin() + 12);
		// and remvoe camera_set from them
		first_twelve.remove_if([camera_a, camera_b, camera_c, camera_d](int a) {
			return (a == camera_a || a == camera_b || a == camera_c || a == camera_d);
		});
		// Assign indirect and double indirect camera_set
		std::vector<int> first_eight(first_twelve.begin(), first_twelve.end());
		ind_camera_set.SetCamerasNoOpt(glm::ivec4(first_eight[0], first_eight[1], first_eight[2], first_eight[3]),
			glm::vec4(1.0 / (1.0001 - ref_camera_dists[first_eight[0]]),
			1.0 / (1.0001 - ref_camera_dists[first_eight[1]]), 
			1.0 / (1.0001 - ref_camera_dists[first_eight[2]]), 
			1.0 / (1.0001 - ref_camera_dists[first_eight[3]])));
		dind_camera_set.SetCamerasNoOpt(glm::ivec4(first_eight[4], first_eight[5], first_eight[6], first_eight[7]),
			glm::vec4(1.0 / (1.0001 - ref_camera_dists[first_eight[4]]),
			1.0 / (1.0001 - ref_camera_dists[first_eight[5]]),
			1.0 / (1.0001 - ref_camera_dists[first_eight[6]]),
			1.0 / (1.0001 - ref_camera_dists[first_eight[7]])));
	}
	else {
		camera_set.SetFaultCameras();
	}
}

void OBJRender::SetVirtualCamera(RenderCamView camera)
{
	SetMatrices(camera.GetViewMatrix(), 
		camera.GetProjectionMatrix(attrib.glnear, attrib.glfar));
	virtual_camera = camera;
	LocateClosestCameras();
}

void OBJRender::SetMatrices(const glm::mat4 &_View, 
	const glm::mat4 &_Projection, const glm::mat4 &_Model)
{
	View = _View;
	Projection = _Projection;
	Model = _Model;
}

void OBJRender::SetRenderLocation(const glm::vec3 &loc)
{
	render_camera_location = loc;
	/* find 4 nearest cameras at the position of eye */
	LocateClosestCameras();
}

void OBJRender::SetLightFieldAttrib(const LightFieldAttrib& attrib)
{
	this->attrib = attrib;
	TransferRefCameraToGL();
}

void OBJRender::GetTextureData(GLuint tex, unsigned char * image)
{
#ifdef GL
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#else 
#warning "OBJRender::GetTextureData not supported with OpenGL ES"
#endif
}

glm::vec3 OBJRender::GetSceneCenter(void)
{
	return scene_center;
}

void OBJRender::SetRenderRes(GLuint width, GLuint height)
{
	targetWidth = width;
	targetHeight = height;
}

vector<int> OBJRender::GetSelectedCamera(void)
{
	vector<int> ret;
	ret.reserve(CLOSEST_CAMERA_NR*3);
	for (int i = 0; i != 4; ++i) {
		ret.push_back(camera_set.GetCamera(i));
	}
	for (int i = 0; i != 4; ++i) {
		ret.push_back(ind_camera_set.GetCamera(i));
	}
	for (int i = 0; i != 4; ++i) {
		ret.push_back(dind_camera_set.GetCamera(i));
	}
	return ret;
}

glm::vec3 OBJRender::CalcObjCenter(void)
{
	glm::vec3 ObjCenter;

	int n = (int)shapes.size();

	ObjCenter.x = 0;
	ObjCenter.y = 0;
	ObjCenter.z = 0;

	int numVertices = 0;

	for (int i = 0; i < n; ++i)
	{
		int group_numVertices = (int)shapes[i].mesh.positions.size() / 3;
		numVertices += group_numVertices;
		for (int j = 0; j < group_numVertices; j++)
		{
			ObjCenter.x += shapes[i].mesh.positions[j * 3];
			ObjCenter.y += shapes[i].mesh.positions[j * 3 + 1];
			ObjCenter.z += shapes[i].mesh.positions[j * 3 + 2];
		}
	}

	ObjCenter.x = ObjCenter.x / numVertices;
	ObjCenter.y = ObjCenter.y / numVertices;
	ObjCenter.z = ObjCenter.z / numVertices;

	return (ObjCenter);
}

GLuint OBJRender::LoadShaders(const char * vertex_code, const char * fragment_code)
{
	//// Read the Vertex Shader code from the file
	//std::string VertexShaderCode;
	//std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	//if (!VertexShaderStream.is_open())
	//{
	//	throw runtime_error("found no vertex shader");
	//}
	//else {
	//	std::string Line = "";
	//	while (getline(VertexShaderStream, Line))
	//		VertexShaderCode += "\n" + Line;
	//	VertexShaderStream.close();
	//}

	//// Read the Fragment Shader code from the file
	//std::string FragmentShaderCode;
	//std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	//if (!FragmentShaderStream.is_open()){
	//	throw runtime_error("found no fragment shader");
	//}
	//else {
	//	std::string Line = "";
	//	while (getline(FragmentShaderStream, Line))
	//		FragmentShaderCode += "\n" + Line;
	//	FragmentShaderStream.close();
	//}

	// Compile shaders
	Shader shader(vertex_code, fragment_code);
	return shader.ID;
}

