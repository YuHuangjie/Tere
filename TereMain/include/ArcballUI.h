#ifndef ARCBALLUI_H
#define ARCBALLUI_H

#include <vector>
#include <string>
#include <memory>

#include "glm/fwd.hpp"
#include "UserInterface.h"

using std::vector;
using std::string;
using std::unique_ptr;

class ArcballUI : public UserInterface
{
public:
	ArcballUI(glm::vec3 last_up, glm::vec3 look_center, const vector<Camera> &refs, 
		const Camera &virCam, const string &cameraMesh);

	virtual std::string Name() const override;
	virtual void Touch(const double x, const double y) override;
	virtual Camera Leave(const double x, const double y, const Camera &view) override;
	virtual Camera Move(const double x, const double y, const Camera &view) override;
	virtual vector<size_t> HintInterp() override;

private:
	glm::vec3 GetArcballVector(double x, double y, const Camera &view);

	glm::vec3 look_center;
	glm::vec3 last_up;

	// UI
	bool arcball_on = false;
	double last_mx, last_my;
	double cur_mx, cur_my;

	// camera mesh
	bool LoadCameraMesh(const string &cameraMeshName);
	vector<size_t> SearchInterpCameras(const Camera &virCam);
	const vector<Camera> &_refCameras;
	unique_ptr<Camera> _pCam;		// active virtual camera

	vector<glm::vec3> camera_vertices;   // camera mesh vertex
	vector<glm::ivec4> camera_quads;      // camera mesh indices
	vector<glm::vec3> ref_camera_dirs;     // center points to each camera
	vector<float> ref_camera_dists;   // distance between center and cameras
	vector<int> ref_camera_index;     // camera sorted by angle distance
};

#endif