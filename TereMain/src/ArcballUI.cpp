#include "glm/gtx/transform.hpp"
#include "ArcballUI.h"
#include "camera/Camera.hpp"

ArcballUI::ArcballUI(glm::vec3 last_up,
	glm::vec3 look_center)
	: UserInterface(),
	last_up(last_up),
	look_center(look_center),
	arcball_on(false),
	last_mx(0.0),
	last_my(0.0),
	cur_mx(0.0),
	cur_my(0.0)
{
	SetResolution(0, 0);
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

	Camera result(Extrinsic(location, location + lookat, up),
		view.GetIntrinsic());

	last_mx = cur_mx;
	last_my = cur_my;

	return result;
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