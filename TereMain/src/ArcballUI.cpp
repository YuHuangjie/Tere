#include <fstream>
#include <sstream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <list>

#include "glm/gtx/transform.hpp"
#include "ArcballUI.h"
#include "camera/Camera.hpp"
#include "common/Log.hpp"
#include "RayTracer.h"

using std::ifstream;
using std::istringstream;
using std::runtime_error;

ArcballUI::ArcballUI(glm::vec3 last_up,	glm::vec3 look_center,
	const vector<Camera> &refs, const Camera &virCam, const string &cameraMesh)
	: UserInterface(),
	last_up(last_up),
	look_center(look_center),
	arcball_on(false),
	last_mx(0.0),
	last_my(0.0),
	cur_mx(0.0),
	cur_my(0.0),
	_refCameras(refs),
	_pCam(new Camera(virCam))
{
	SetResolution(0, 0);

	if (!LoadCameraMesh(cameraMesh)) {
		throw runtime_error(string("failed loading camera mesh: ") + cameraMesh);
	}
}

void ArcballUI::Touch(const double x, const double y)
{
	arcball_on = true;
	last_mx = x;
	last_my = y;
}

Camera ArcballUI::Leave(const double x, const double y, const Camera &view)
{
	arcball_on = false;
	last_mx = x;
	last_my = y;
	*_pCam = view;
	return view;
}

Camera ArcballUI::Move(const double x, const double y, const Camera &view)
{
	if (!arcball_on) { return view; }

	cur_mx = x;
	cur_my = y;

	if (cur_mx == last_mx && cur_my == last_my) { return view; }

	glm::vec3 va = GetArcballVector(last_mx, last_my, view);
	glm::vec3 vb = GetArcballVector(cur_mx, cur_my, view);
	float angle = glm::acos(glm::min(1.0f, glm::dot(va, vb))) * 1.5f;
	glm::vec3 axis_in_camera = glm::cross(va, vb);
	glm::mat4 trans(1.0f);
	trans = glm::rotate(trans, angle, axis_in_camera);

	glm::vec4 view_dir = glm::vec4(view.GetPosition(), 1) - 
		glm::vec4(look_center, 1);
	glm::vec4 _location = glm::transpose(trans) * view_dir + 
		glm::vec4(look_center, 1);

	glm::vec3 location(_location);
	glm::vec3 lookat = glm::normalize(look_center - location);
	glm::vec3 right = glm::cross(lookat, last_up);
	glm::vec3 up = glm::cross(right, lookat);
	lookat = glm::normalize(lookat);
	up = glm::normalize(up);
	last_up = up;

	_pCam->SetExtrinsic(Extrinsic(location, location + lookat, up));
	_pCam->SetIntrinsic(view.GetIntrinsic());

	last_mx = cur_mx;
	last_my = cur_my;

	return *_pCam;
}

glm::vec3 ArcballUI::GetArcballVector(double x, double y, const Camera &view)
{
	glm::vec3 P(1.0 * x / screen_width * 2 - 1.0,
		1.0 * y / screen_height * 2 - 1.0,
		0);

	P.y = -P.y;
	float op_squared = P.x * P.x + P.y * P.y;
	if (op_squared < 1.0) {
		P.z = sqrt(1.0f - op_squared);
	}
	else {
		P = glm::normalize(P);
	}

	P = view.GetRight() * P.x + view.GetUp() * P.y + view.GetDir() * P.z;

	return P;
}

std::string ArcballUI::Name() const
{
	return "arcball";
}

vector<size_t> ArcballUI::HintInterp()
{
	return SearchInterpCameras(*_pCam);
}

bool ArcballUI::LoadCameraMesh(const string &cameraMeshName)
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

vector<size_t> ArcballUI::SearchInterpCameras(const Camera &virCam)
{
	// Prepare directions from reference camera to scene_center
	if (ref_camera_dirs.empty()) {
		for (auto ref : _refCameras)
		{
			ref_camera_dirs.push_back(glm::normalize(ref.GetPosition() -
				look_center));
			ref_camera_dists.push_back(2.0f);
		}
		ref_camera_index = vector<int>(ref_camera_dirs.size());
		std::iota(ref_camera_index.begin(), ref_camera_index.end(), 0);
	}

	// Ray-quad intersection
	bool intersected = false;
	int intersected_quad_id = -1;
	glm::vec3 origin = look_center;
	glm::vec3 view_vec = virCam.GetDir();

	intersected_quad_id = RayQuadIntersect(camera_vertices, camera_quads,
		origin, view_vec);

	if (intersected_quad_id >= 0) {
		intersected = true;
	}

	vector<size_t> indices;

	if (intersected) {
		// directly intersected quad
		for (int i = 0; i != 4; ++i) {
			int index = camera_quads[intersected_quad_id][i];
			if (index < 0) {
				continue;
			}
			indices.push_back(index);
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

		// extract first twelve indices
		std::list<int> first_twelve(ref_camera_index.begin(), ref_camera_index.begin() + how_many_sort);
		// and remvoe already interpolated from them
		first_twelve.remove_if([&indices](int a) {
			bool flag = (a == indices[0] || a == indices[1] ||
				a == indices[2]);
			flag |= (indices.size() > 3 ? a == indices[3] : false);
			return flag;
		});

		// eight indices
		std::list<int>::const_iterator itEight = first_twelve.cbegin();

		for (int i = 0; i != how_many_sort - 4; ++i) {
			int index = *itEight++;
			indices.push_back(index);
		}
	}

	return indices;
}