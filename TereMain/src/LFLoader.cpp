#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <future>
#include "LFLoader.h"
#include "OBJRender.h"
#include "common/CommonIO.hpp"
#include "image/ImageIO.hpp"

LFLoader::LFLoader(const string &profileFile) 
	: attrib()
{
	// read profile, such as camera pose, intrinsic and image list
	ifstream in_profile(profileFile);
	if (!in_profile.is_open()) {
		throw runtime_error("cannot open profile");
	}

	// compute profile prefix
	string::size_type prefix_pos = profileFile.find_last_of("/\\");
	string profile_prefix = "";

	if (prefix_pos == string::npos) {
		profile_prefix = "";
	}
	else {
		profile_prefix = profileFile.substr(0, prefix_pos) + "/";
	}

	// profile entry should be of this format: [key: value]
	map<string, string> profile;

	while (!in_profile.eof()) {
		string key, value, line;

		getline(in_profile, line);
		if (line.empty()) continue;
		if (line[0] == '#') continue;

		string::size_type delim_pos = line.find(":");
		if (delim_pos == string::npos) {
			continue;
		}

		key = line.substr(0, delim_pos);
		value = line.substr(delim_pos + 1);
		// remove white spaces
		key.erase(key.find_last_not_of(" \n\r\t") + 1);
		key.erase(0, key.find_first_not_of(" \n\r\t"));
		value.erase(value.find_last_not_of(" \n\r\t") + 1);
		value.erase(0, value.find_first_not_of(" \n\r\t"));

		profile.insert( pair<string, string>(key, value) );
	}

	// release resource
	in_profile.close();

	// Load configurations
	LoadProfile(profile_prefix, profile);
}

void LFLoader::LoadProfile(const string &profile_prefix,
	const map<string, string> &profile)
{
	// read image list
	auto search = profile.find("image_list");
	string image_list_file = "";
	if (search != profile.end()) { image_list_file = profile_prefix + search->second; }
	else {	throw runtime_error("no image_list found in profile"); }

	CommonIO::ReadList(image_list_file, attrib.image_list, true);

	// read image size
	search = profile.find("width_H");
	if (search != profile.end()) { 
		attrib.width_H = atoi((search->second).c_str()); 
	}
	else { throw runtime_error("no width_H found in profile"); }

	search = profile.find("height_H");
	if (search != profile.end()) { 
		attrib.height_H = atoi((search->second).c_str()); 
	}
	else { throw runtime_error("no height_H found in profile"); }

	search = profile.find("width_L");
	if (search != profile.end()) { 
		attrib.width_L = atoi((search->second).c_str());
	}
	else { throw runtime_error("no width_L found in profile"); }

	search = profile.find("height_L");
	if (search != profile.end()) { 
		attrib.height_L = atoi((search->second).c_str());
	}
	else { throw runtime_error("no height_L found in profile"); }

	// read light field constants
	search = profile.find("N_REF_CAMERAS");
	if (search != profile.end()) { 
		attrib.N_REF_CAMERAS = atoi(search->second.c_str());
	}
	else { throw runtime_error("no N_REF_CAMERAS found in profile"); }

	/* read near, far */
	search = profile.find("near");
	if (search != profile.end()) {
		attrib.glnear = (float)atof(search->second.c_str());
	}
	else { throw runtime_error("no near found in profile"); }

	search = profile.find("far");
	if (search != profile.end()) {
		attrib.glfar = (float)atof(search->second.c_str());
	}
	else { throw runtime_error("no far found in profile"); }

	// read reference cameras
	string pose_filename = "";
	string K_filename = "";
	search = profile.find("camera_pose");
	if (search != profile.end()) { 
		pose_filename = profile_prefix + search->second; }
	else { throw runtime_error("no camera_pose found in profile"); 
	}

	search = profile.find("camera_intrinsic");
	if (search != profile.end()) { 
		K_filename = profile_prefix + search->second; }
	else { throw runtime_error("no camera_intrinsic found in profile"); 
	}

	std::vector<Extrinsic> poses;
	std::vector<Intrinsic> ks;
	int nPoses = CommonIO::ReadExtrinsic(pose_filename, poses);
	int nKs = CommonIO::ReadIntrinsic(K_filename, ks);

	for (int i = 0; i < nPoses && i < nKs; ++i) {
		Camera refcamera(Camera(poses[i], ks[i]));
		attrib.ref_cameras.push_back(refcamera);
		attrib.ref_cameras_V.push_back(refcamera.GetViewMatrix());
		attrib.ref_cameras_VP.push_back
			(refcamera.GetProjectionMatrix(attrib.glnear, 
				attrib.glfar) * refcamera.GetViewMatrix());
	}

	// read reference camera center and radius
	search = profile.find("camera_center");
	if (search != profile.end()) {
		glm::vec3 center;
		istringstream iss(search->second);
		iss >> center.x >> center.y	>> center.z;
		attrib.ref_camera_center = center;
	}
	else { throw runtime_error("no camera_center found in profile"); }

	search = profile.find("camera_radius");
	if (search != profile.end()) {
		istringstream iss(search->second);
		iss >> attrib.ref_camera_radius;
	}
	else { throw runtime_error("no camera_radius found in profile"); }

	// check light field configuration is consistent
	if (attrib.ref_cameras.size() != attrib.N_REF_CAMERAS) {
		throw runtime_error("light field configration is inconsistent");
	}

	/* read obj name */
	search = profile.find("obj");
	if (search != profile.end()) { 
		attrib.obj_file = profile_prefix + search->second; 
	}
	else { throw runtime_error("no obj file found"); }

	/* read reference camera mesh obj name */
	search = profile.find("camera_mesh");
	if (search != profile.end()) { 
		attrib.camera_mesh_name = profile_prefix + search->second; 
	}
	else { throw runtime_error("no camera_mesh file found"); }
}

bool LFLoader::OnDecompressing(const int thread_id, const int thread_nr)
{
	int image_num = attrib.N_REF_CAMERAS;
	int texture_width = attrib.width_L;
	int texture_height = attrib.height_L;

	for (int i = 0; i != image_num; ++i) {
		// distribute mission to different threads
		if (i % thread_nr != thread_id) { continue; }

		// Read compressed texture
		Image image = ImageIO::Read(attrib.image_list[i], texture_width,
			texture_height);

		if (!image.GetData()) {
			return false;
		}

		std::memcpy(rgbBuffer.data() + i*texture_width*texture_height * 3,
			image.GetData(), texture_height * texture_width * 3);
	}

	return true;
}

bool LFLoader::Decompress(const int no_thread)
{
	// allocate memory for rgb textures
	int texture_width = attrib.width_L;
	int texture_height = attrib.height_L;
	std::vector< std::future<bool> > tasks(no_thread);

	// some versions of libjpeg-turbo has a bug, hence the odd +1 here
	rgbBuffer = vector<uint8_t>(texture_width*texture_height * 3 *
		attrib.N_REF_CAMERAS + 1, 0);

	for (int i = 0; i != no_thread; ++i) {
		tasks[i] = std::async(std::launch::async, &LFLoader::OnDecompressing, this, i, no_thread);
	}

	for (int i = 0; i != no_thread; ++i) {
		if (!tasks[i].valid() || !tasks[i].get()) {
			return false;
		}
	}

	return true;
}

vector<GLuint> LFLoader::GenerateRGBDTextures(const unique_ptr<OBJRender> &renderer)
{
	int image_num = attrib.N_REF_CAMERAS;
	int texture_width = attrib.width_L;
	int texture_height = attrib.height_L;
	vector<GLuint> light_field_tex(image_num, 0);

	glGenTextures(image_num, light_field_tex.data());

	for (int i = 0; i != image_num; ++i) {
		// upload RGB image
		GLuint rgb = 0;
		glGenTextures(1, &rgb);
		glBindTexture(GL_TEXTURE_2D, rgb);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, texture_width, texture_height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height,
			GL_RGB, GL_UNSIGNED_BYTE,
			rgbBuffer.data() + i*texture_width*texture_height * 3);
		glBindTexture(GL_TEXTURE_2D, 0);

		// append depth channel to RGB texture
		GLuint rgbd = renderer->AppendDepth(rgb,
			texture_width, texture_height,
			attrib.ref_cameras_VP[i], 
			attrib.ref_cameras_V[i]);
		
		light_field_tex[i] = rgbd;

		// delete rgb texture
		glDeleteTextures(1, &rgb);

		// glTexSubImage2D allocate CPU memory and copy the user's data into it.
		// That increase much memory misallocating. An approch is to synchronize
		// the texture uploading process.
		// \cite{https://www.khronos.org/opengl/wiki/Synchronization}.
		glFinish();
	}

	// Clear CPU image memory
	rgbBuffer = vector<uint8_t>();

	return light_field_tex;
}
