#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include "LFLoader.h"
#include "common/CommonIO.hpp"

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
