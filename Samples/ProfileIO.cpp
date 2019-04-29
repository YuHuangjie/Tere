#include <sstream>
#include <cstdint>
#include <fstream>
#include <map>
#include <iostream>
#include <algorithm>
#include <cctype>

#include "ProfileIO.hpp"
#include "Assert.h"

using namespace std;

/* Extrinsic entry is cam2world, it has the following format:
*	 id x[0] x[1] x[2] t[0] y[0] y[1] y[2] t[1] z[0] z[1] z[2] t[2] 0 0 0 1
*    where x = right, y = up, z = dir
*/
static void ParseExtrinsic(const string &s, array<float, 16> &M)
{
	istringstream iss(s);
	iss >> M[0] >> M[1] >> M[2] >> M[3]
		>> M[4] >> M[5] >> M[6] >> M[7]
		>> M[8] >> M[9] >> M[10] >> M[11]
		>> M[12] >> M[13] >> M[14] >> M[15];
}

/* Intrinsic entry has the following format: 
 *   id cx 0 fx 0 cy fy 0 0 1
 */
static void ParseIntrinsic(const string &s, array<float, 9> &M)
{
	istringstream iss(s);
	
	iss >> M[0] >> M[1] >> M[2]
		>> M[3] >> M[4] >> M[5]
		>> M[6] >> M[7] >> M[8];
}

static inline void ltrim(string &s) {
	s.erase(s.begin(), std::find_if(s.cbegin(), s.cend(), [](int ch) {
		return !std::isspace(ch);
	}));
}

static inline void rtrim(string &s) {
	s.erase(std::find_if(s.crbegin(), s.crend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

static inline void trim(string &s) {
	ltrim(s);
	rtrim(s);
}

static inline string trim_copy(string s) {
	trim(s);
	return s;
}

/**
* Read list file which have the following format:
*	<index:int>	<value:string>
* Entry with same index will overwrite previous entry
* empty string is set if no such index is found
*
* @param prefix		Prefix will be added to final value. For example, say
*					prefix="c:/", and value is "0000.jpg", the final value
*					will be "c:/0000.jpg".
* @param filename	The name of list file
* @param container	A string container to store list values
*
* @return The number of entries found
*/
static int ReadList(const string &filename, const string &prefix, 
	vector<string>& list)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) {
		throw runtime_error("[Error] ProfileIO: Can't open " + filename);
	}

	int maxIndex = 0;
	int nEntries = 0;

	while (!inFile.eof()) {
		int index;
		string line;
		istringstream iss;
		char value[1024];

		std::getline(inFile, line);
		if (line.empty()) continue;
		if (line.find_first_not_of(" \r\n\t") == std::string::npos) {
			continue;
		}
		if (line[0] == '#') continue;

		iss.str(line);
		iss >> index;
		iss.getline(value, sizeof(value));

		if (index > maxIndex) {
			maxIndex = index;
		}
		while (index + 1 > static_cast<int>(list.size())) {
			list.resize(list.size() * 2 + 1);
		}

		list[index] = prefix + trim_copy(string(value));
		nEntries++;
	}

	list.resize(maxIndex + 1);

	return nEntries;
}


static void ParseProfile(const string &prefix,
	const map<string, string> &profile, Profile &p)
{
	size_t nCams;
	string imageListF;
	string intrinListF, extrinListF;
	string meshF;
	string _mode;
	RENDER_MODE mode;
	size_t rows;
	vector<string> imageList, intrinList, extrinList;

	for (auto it = profile.cbegin(); it != profile.cend(); ++it) {
		if (it->first == "image_list") {
			imageListF = prefix + it->second;
		}
		else if (it->first == "N_REF_CAMERAS") {
			nCams = atoi(it->second.c_str());
		}
		else if (it->first == "camera_pose") {
			extrinListF = prefix + it->second;
		}
		else if (it->first == "camera_intrinsic") {
			intrinListF = prefix + it->second;
		}
		else if (it->first == "obj") {
			meshF = prefix + it->second;
		}
		else if (it->first == "mode") {
			_mode = it->second;
		}
		else if (it->first == "rows") {
			rows = atoi(it->second.c_str());
		}
		else {
			cout << "Unknown profile key" << endl;
		}
	}

	// read image list
	ASSERT(ReadList(imageListF, prefix, imageList) == nCams);

	// read extrinsics
	ASSERT(ReadList(extrinListF, "", extrinList) == nCams);

	vector<array<float, 16>> extrins(nCams);
	for (size_t i = 0; i < nCams; ++i) { ParseExtrinsic(extrinList[i], extrins[i]); }

	// read intrinsics
	ASSERT(ReadList(intrinListF, "", intrinList) == nCams);

	vector<array<float, 9>> intrins(nCams);
	for (size_t i = 0; i < nCams; ++i) { ParseIntrinsic(intrinList[i], intrins[i]); }

	if (_mode == "linear") mode = RENDER_MODE::LINEAR;
	else if (_mode == "arcball") mode = RENDER_MODE::SPHERE;
	if (_mode == "all") mode = RENDER_MODE::ALL;

	p.nCams = nCams;
	p.extrins = extrins;
	p.intrins = intrins;
	p.imageList = imageList;
	p.mesh = meshF;
	p.mode = mode;
	p.rows = rows;

	//// read image list
	//auto search = profile.find("image_list");
	//string image_list_file = "";
	//if (search != profile.end()) { image_list_file = prefix + search->second; }
	//else { throw runtime_error("no image_list found in profile"); }

	//ReadList(image_list_file, list, true);

	// read image size
	//search = profile.find("width_H");
	//if (search != profile.end()) {
	//	attrib.width_H = atoi((search->second).c_str());
	//}
	//else { throw runtime_error("no width_H found in profile"); }

	//search = profile.find("height_H");
	//if (search != profile.end()) {
	//	attrib.height_H = atoi((search->second).c_str());
	//}
	//else { throw runtime_error("no height_H found in profile"); }

	//search = profile.find("width_L");
	//if (search != profile.end()) {
	//	attrib.width_L = atoi((search->second).c_str());
	//}
	//else { throw runtime_error("no width_L found in profile"); }

	//search = profile.find("height_L");
	//if (search != profile.end()) {
	//	attrib.height_L = atoi((search->second).c_str());
	//}
	//else { throw runtime_error("no height_L found in profile"); }

	// read light field constants
	//search = profile.find("N_REF_CAMERAS");
	//if (search != profile.end()) {
	//	attrib.N_REF_CAMERAS = atoi(search->second.c_str());
	//}
	//else { throw runtime_error("no N_REF_CAMERAS found in profile"); }

	/* read near, far */
	//search = profile.find("near");
	//if (search != profile.end()) {
	//	attrib.glnear = (float)atof(search->second.c_str());
	//}
	//else { throw runtime_error("no near found in profile"); }

	//search = profile.find("far");
	//if (search != profile.end()) {
	//	attrib.glfar = (float)atof(search->second.c_str());
	//}
	//else { throw runtime_error("no far found in profile"); }

	// read reference cameras
	//string pose_filename = "";
	//string K_filename = "";
	//search = profile.find("camera_pose");
	//if (search != profile.end()) {
	//	pose_filename = prefix + search->second;
	//}
	//else {
	//	throw runtime_error("no camera_pose found in profile");
	//}

	//search = profile.find("camera_intrinsic");
	//if (search != profile.end()) {
	//	K_filename = prefix + search->second;
	//}
	//else {
	//	throw runtime_error("no camera_intrinsic found in profile");
	//}

	//std::vector<Extrinsic> poses;
	//std::vector<Intrinsic> ks;
	//int nPoses = CommonIO::ReadExtrinsic(pose_filename, poses);
	//int nKs = CommonIO::ReadIntrinsic(K_filename, ks);

	//for (int i = 0; i < nPoses && i < nKs; ++i) {
	//	Camera refcamera(Camera(poses[i], ks[i]));
	//	attrib.ref_cameras.push_back(refcamera);
	//	attrib.ref_cameras_V.push_back(refcamera.GetViewMatrix());
	//	attrib.ref_cameras_VP.push_back
	//	(refcamera.GetProjectionMatrix(attrib.glnear,
	//		attrib.glfar) * refcamera.GetViewMatrix());
	//}

	// read reference camera center and radius
	//search = profile.find("camera_center");
	//if (search != profile.end()) {
	//	glm::vec3 center;
	//	istringstream iss(search->second);
	//	iss >> center.x >> center.y >> center.z;
	//	attrib.ref_camera_center = center;
	//}
	//else { throw runtime_error("no camera_center found in profile"); }

	//search = profile.find("camera_radius");
	//if (search != profile.end()) {
	//	istringstream iss(search->second);
	//	iss >> attrib.ref_camera_radius;
	//}
	//else { throw runtime_error("no camera_radius found in profile"); }

	// check light field configuration is consistent
	//if (attrib.ref_cameras.size() != attrib.N_REF_CAMERAS) {
	//	throw runtime_error("light field configration is inconsistent");
	//}

	/* read obj name */
	//search = profile.find("obj");
	//if (search != profile.end()) {
	//	attrib.obj_file = prefix + search->second;
	//}
	//else { throw runtime_error("no obj file found"); }

	// read ui mode
	//search = profile.find("mode");
	//if (search != profile.end()) {
	//	attrib._uiMode = search->second;
	//}
	//else { throw runtime_error("no ui mode found"); }

	// ui sub-parameters
	//if (attrib._uiMode == "linear") {
	//	search = profile.find("rows");
	//	if (search != profile.end()) {
	//		attrib._rows = std::atoi(search->second.c_str());
	//	}
	//	else { throw runtime_error("no rows found when mode=linear"); }
	//}
	//else if (attrib._uiMode == "arcball") {
	//	search = profile.find("camera_mesh");
	//	if (search != profile.end()) {
	//		attrib.camera_mesh_name = prefix + search->second;
	//	}
	//	else { throw runtime_error("no camera_mesh file found"); }
	//}
	//else {
	//	throw runtime_error("unknown ui mode");
	//}
}

Profile ReadProfile(const std::string &profileFile)
{
	Profile p;
	ifstream in_profile(profileFile);

	// read profile, such as camera pose, intrinsic and image list
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

		profile.insert(pair<string, string>(key, value));
	}

	// close profile
	in_profile.close();

	// Load configurations
	ParseProfile(profile_prefix, profile, p);

	return p;
}

static int WriteList(const string &filename, const vector<string> &list)
{
	ofstream outFile(filename);
	int index = 0;

	if (!outFile.is_open()) {
		throw runtime_error("[Error] ProfileIO: Can't open " + filename);
	}

	for (auto it = list.cbegin(); it != list.cend(); ++it) {
		outFile << index << " " << *it << endl;
		index++;
	}

	outFile.close();
	return index;
}

//int WriteIntrinsic(const std::string &filename,
//	const vector<Intrinsic> &container)
//{
//	ofstream outFile(filename);
//	if (!outFile.is_open()) {
//		throw runtime_error("!![Error] ProfileIO: Can't open " + filename);
//	}
//
//	int index = 0;
//
//	// comment
//	outFile << "## index fx fy cx cy width height" << endl;
//
//	for (vector<Intrinsic>::const_iterator it = container.cbegin();
//		it != container.cend(); ++it) {
//		outFile << index << " " << it->GetFx() << " " << it->GetFy() << " "
//			<< it->GetCx() << " " << it->GetCy() << " " << it->GetWidth() << " "
//			<< it->GetHeight() << endl;
//		index++;
//	}
//
//	outFile.close();
//	return index;
//}
//
//
//
//int WriteExtrinsic(const std::string &filename,
//	const vector<Extrinsic> &container)
//{
//	ofstream outFile(filename);
//	if (!outFile.is_open()) {
//		throw runtime_error("!![Error] ProfileIO: Can't open " + filename);
//	}
//
//	int index = 0;
//
//	// comment
//	outFile << "## index up dir position" << endl;
//
//	for (vector<Extrinsic>::const_iterator it = container.cbegin();
//		it != container.cend(); ++it) {
//		outFile << index << " " << it->GetUp().x << " " << it->GetUp().y
//			<< " " << it->GetUp().z << " " << it->GetDir().x << " "
//			<< it->GetDir().y << " " << it->GetDir().z << " "
//			<< it->GetPos().x << " " << it->GetPos().y << " "
//			<< it->GetPos().z << endl;
//		index++;
//	}
//
//	outFile.close();
//	return index;
//}