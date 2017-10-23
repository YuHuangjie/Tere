#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include "LFLoader.h"
#include "OBJRender.h"
#include "camera.h"
#include "Decoder.h"

LFLoader::~LFLoader()
{
	glDeleteTextures(light_field_attrib.N_REF_CAMERAS, light_field_attrib.light_field_tex);
	delete[] light_field_attrib.light_field_tex;
}

LFLoader::LFLoader(const string &profile) 
	: glnear(0.f), glfar(0.f)
{
	// read profile, such as camera pose, intrinsic and preset
	ifstream in_profile(profile);
	if (!in_profile.is_open()) {
		throw runtime_error("cannot open profile file");
	}

	/* try determine image prefix and size */
	string::size_type prefix_pos = profile.find_last_of("/\\");
	profile_prefix = profile.substr(0, prefix_pos) + "/";

	// profile entry shoulf of this format: [key: value]
	while (!in_profile.eof()) {
		string key, value, line;

		getline(in_profile, line);
		if (line.empty()) continue;
		if (line[0] == '#') continue;

		string::size_type delim_pos = line.find(":");
		key = line.substr(0, delim_pos);
		value = line.substr(delim_pos + 2);
		key.erase(key.find_last_not_of(" \n\r\t") + 1);
		value.erase(value.find_last_not_of(" \n\r\t") + 1);
		this->profile.insert( pair<string, string>(key, value) );
	}

	// release resource
	in_profile.close();

	// Load configurations
	LoadProfile();
}

void LFLoader::LoadProfile(void)
{
	/* try read image list configuration file */
	auto search = profile.find("image_list");
	string image_list_file = "";
	if (search != profile.end()) { image_list_file = profile_prefix + search->second; }
	else {	throw runtime_error("no image_list found in profile"); }
	ifstream in_image(image_list_file);
	if (!in_image.is_open()) {
		throw runtime_error("cannot open image_list file");
	}
	string image_idx;
	string image_name;
	while (in_image >> image_idx >> image_name) {
		image_files.push_back(image_name);
	}
	if (image_files.size() == 0) {
		throw runtime_error("no images found");
	}
	light_field_attrib.image_list = image_files;

	/* try determine image prefix and size */
	light_field_attrib.image_file_prefix = profile_prefix;
	image_file_prefix = profile_prefix;

	search = profile.find("width_H");
	if (search != profile.end()) { image_width_H = atoi((search->second).c_str()); }
	else { throw runtime_error("no width_H found in profile"); }
	search = profile.find("height_H");
	if (search != profile.end()) { image_height_H = atoi((search->second).c_str()); }
	else { throw runtime_error("no height_H found in profile"); }
	search = profile.find("width_L");
	if (search != profile.end()) { image_width_L = atoi((search->second).c_str()); }
	else { throw runtime_error("no width_L found in profile"); }
	search = profile.find("height_L");
	if (search != profile.end()) { image_height_L = atoi((search->second).c_str()); }
	else { throw runtime_error("no height_L found in profile"); }
	light_field_attrib.width_H = image_width_H;
	light_field_attrib.height_H = image_height_H;
	light_field_attrib.width_L = image_width_L;
	light_field_attrib.height_L = image_height_L;


	/* locate vertex shader and fragment shader */
	//search = profile.find("scene_vertex_shader");
	//if (search != profile.end()) { light_field_attrib.scene_vs_file = profile_prefix + search->second; }
	//else { throw runtime_error("no vertex_shader found in profile"); }
	//search = profile.find("scene_frag_shader");
	//if (search != profile.end()) { light_field_attrib.scene_frag_file = profile_prefix + search->second; }
	//else { throw runtime_error("no frag_shader found in profile"); }
	//search = profile.find("depth_vertex_shader");
	//if (search != profile.end()) { light_field_attrib.depth_vs_file = profile_prefix + search->second; }
	//else { throw runtime_error("no vertex_shader found in profile"); }
	//search = profile.find("depth_frag_shader");
	//if (search != profile.end()) { light_field_attrib.depth_frag_file = profile_prefix + search->second; }
	//else { throw runtime_error("no frag_shader found in profile"); }

	/* try read light stage constants */
	search = profile.find("N_REF_CAMERAS_HIGH");
	if (search != profile.end()) { image_num_high_res = atoi(search->second.c_str()); }
	else { throw runtime_error("no N_REF_CAMERAS_HIGH found in profile"); }
	search = profile.find("N_REF_CAMERAS_LOW");
	if (search != profile.end()) { image_num_low_res = atoi(search->second.c_str()); }
	else { throw runtime_error("no N_REF_CAMERAS_LOW found in profile"); }
	light_field_attrib.N_REF_CAMERAS = (int)image_num_high_res + (int)image_num_low_res;
	light_field_attrib.N_REF_CAMERAS_HIGH = (int)image_num_high_res;
	light_field_attrib.N_REF_CAMERAS_LOW = (int)image_num_low_res;

	/* read near, far */
	search = profile.find("near");
	if (search != profile.end()) { glnear = (float)atof(search->second.c_str()); }
	else { throw runtime_error("no near found in profile"); }
	search = profile.find("far");
	if (search != profile.end()) { glfar = (float)atof(search->second.c_str()); }
	else { throw runtime_error("no far found in profile"); }
	light_field_attrib.glnear = glnear;
	light_field_attrib.glfar = glfar;

	/* try read reference cameras */
	string pose_filename = "";
	string K_filename = "";
	ifstream pose_file, K_file;
	CamPose pose;
	CamIntrinsic K;
	RenderCamView ref_camera;
	search = profile.find("camera_pose");
	if (search != profile.end()) { pose_filename = profile_prefix + search->second; }
	else { throw runtime_error("no camera_pose found in profile"); }
	search = profile.find("camera_intrinsic");
	if (search != profile.end()) { K_filename = profile_prefix + search->second; }
	else { throw runtime_error("no camera_intrinsic found in profile"); }
	pose_file.open(pose_filename);
	K_file.open(K_filename);
	if (!pose_file.is_open()) { throw runtime_error("cannot open camera_pose file"); }
	if (!K_file.is_open()) { throw runtime_error("cannot open camera_intrinsic file"); }
	if (glnear < 0.1 || glfar < 0.1) { throw runtime_error("near far is illegal"); }
	while (!pose_file.eof() && !K_file.eof() 
		&& light_field_attrib.ref_cameras.size() != light_field_attrib.N_REF_CAMERAS) {
		
		pose.Read(pose_file);
		K.Read(K_file);
		ref_camera.setParameter(pose.GetOffset(), pose.GetLookat(), pose.GetUp(),
			K.GetFoV().x, K.GetFoV().y, K.GetCx(), K.GetCy(), image_width_H, image_height_H);
		light_field_attrib.ref_cameras.push_back(ref_camera);
		light_field_attrib.ref_cameras_V.push_back(ref_camera.GetViewMatrix());
		light_field_attrib.ref_cameras_VP.push_back
			(ref_camera.GetProjectionMatrix(glnear, glfar) * ref_camera.GetViewMatrix());
	}

	/* try reading reference camera center and radius */
	search = profile.find("camera_center");
	if (search != profile.end()) {
		istringstream iss(search->second);
		iss >> ref_camera_center.x >> ref_camera_center.y >> ref_camera_center.z;
		light_field_attrib.ref_camera_center = ref_camera_center;
	}
	else { throw runtime_error("no camera_center found in profile"); }
	search = profile.find("camera_radius");
	if (search != profile.end()) {
		istringstream iss(search->second);
		iss >> ref_camera_radius;
		light_field_attrib.ref_camera_radius = ref_camera_radius;
	}
	else { throw runtime_error("no camera_radius found in profile"); }

	/* checking light field configuration is consistent */
	if (light_field_attrib.ref_cameras.size() != light_field_attrib.N_REF_CAMERAS) {
		throw runtime_error("light field configration is inconsistent");
	}

	/* read obj name */
	search = profile.find("obj");
	if (search != profile.end()) { light_field_attrib.obj_file = profile_prefix + search->second; }
	else { throw runtime_error("no obj file found"); }

	/* read reference camera mesh obj name */
	search = profile.find("camera_mesh");
	if (search != profile.end()) { light_field_attrib.camera_mesh_name = profile_prefix + search->second; }
	else { throw runtime_error("no camera_mesh file found"); }

	/* read lookup table */
	//search = profile.find("preset");
	//string lookup_table_name = "";
	//if (search != profile.end()) { lookup_table_name = search->second; }
	//else { throw runtime_error("no preset found in profile"); }
	//ifstream in_preset(lookup_table_name);
	//if (!in_preset.is_open()) { throw runtime_error("cannot open lookup table"); }

	/* release resources */
	in_image.close();
	//in_preset.close();
	pose_file.close();
	K_file.close();
	//in_image_H.close();
}

void LFLoader::PreDecompress(void)
{
	/* Generate textures */
	int image_num = light_field_attrib.N_REF_CAMERAS;
	light_field_attrib.light_field_tex = new GLuint[image_num];
	glGenTextures(image_num, light_field_attrib.light_field_tex);

	// allocate memory for rgb textures
	rgb_buffer = new unsigned char[image_width_L*image_height_L * 3 * light_field_attrib.N_REF_CAMERAS];
}

void LFLoader::PostDecompress(void)
{
	int image_num = light_field_attrib.N_REF_CAMERAS;
	for (int i = 0; i != image_num; ++i) {
		glBindTexture(GL_TEXTURE_2D, light_field_attrib.light_field_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width_L, image_height_L,
			0, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer+i*image_width_L*image_height_L*3);
	}

	delete[] rgb_buffer;
	rgb_buffer = nullptr;
}

void LFLoader::OnDecompressing(const int thread_id, const int thread_nr)
{
	int image_num = light_field_attrib.N_REF_CAMERAS;
	string image_file;
	// Some versions of libjpeg-turbo definitely have a bug. Thus the odd '+1' here.
	unsigned char * buf = new unsigned char[image_width_L * image_height_L * 3 + 1];

	for (int i = 0; i != image_num; ++i) {
		// distribute mission to different threads
		if (i % thread_nr != thread_id) { continue; }

		image_file = image_file_prefix + image_files[i];
		try {
			::Decompress(image_file, buf, image_width_L, image_height_L);
		}
		catch (std::runtime_error &e) {
			throw e;
		}
		memcpy(rgb_buffer + i*image_width_L*image_height_L * 3,
			buf, image_width_L*image_height_L * 3);
	}

	// release resource
	delete[] buf;
	buf = nullptr;
}

void LFLoader::Decompress(const int no_thread)
{
	std::vector<std::thread> t(no_thread);
	for (int i = 0; i != no_thread; ++i) {
		t[i] = std::thread(&LFLoader::OnDecompressing, this, i, no_thread);
	}
	for (int i = 0; i != no_thread; ++i)
		if (t[i].joinable())
			t[i].join();
}

void LFLoader::GenerateRGBDTexture(const unique_ptr<OBJRender> &renderer)
{
	int image_num = light_field_attrib.N_REF_CAMERAS;

	for (int i = 0; i != image_num; ++i) {
		// generate RGBD textures
		GLuint rgbd = renderer->RenderRGBD(i, light_field_attrib.light_field_tex[i],
			light_field_attrib.width_L, light_field_attrib.height_L);
		
		// replace old texture
		glDeleteTextures(1, &light_field_attrib.light_field_tex[i]);
		light_field_attrib.light_field_tex[i] = rgbd;
	}
}

void LFLoader::LoadPreset(void)
{
	ifstream in_file(profile["preset"], ifstream::binary);
	if (!in_file.is_open()) {
		throw runtime_error("could not open lookup table file");
	}

	// get stream length
	in_file.seekg(0, ifstream::end);
	int length_in_byte = (int)in_file.tellg();
	in_file.seekg(ifstream::beg);

	if (length_in_byte % 2 != 0) {
		throw runtime_error("lookup table is incorrect");
	}

	// read all data as bytes
	char *buf = new char[length_in_byte];
	in_file.read(buf, length_in_byte);

	// restore buffer as integers
	short *data = reinterpret_cast<short*>(buf);
	int length_in_short = length_in_byte / 2;
	lookup_table = vector<int>(length_in_short, 0);

	for (int i = 0; i != length_in_short; ++i) {
		lookup_table[i] = data[i];
	}
	
	// lookup table memory usage
	int usage = (int)lookup_table.size() * sizeof(int) / 1024;
	LOGD("lookup table occupied %d KB memory\n", usage);

	/* release resource */
	delete[] buf;

}
