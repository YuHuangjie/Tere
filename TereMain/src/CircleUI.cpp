#include "CircleUI.h"
#include "camera/Camera.hpp"

CircleUI::CircleUI(const glm::vec3 & up, const glm::vec3 & center)
	: UserInterface(),
	_up(up),
	_center(center),
	_activated(false),
	_px(0.0),
	_py(0.0),
	_cx(0.0),
	_cy(0.0)
{
	SetResolution(0, 0);
}

void CircleUI::Touch(const double x, const double y)
{
	_activated = true;
	_px = x;
}

Camera CircleUI::Leave(const double x, const double y, const Camera &view)
{
	_activated = false;
	_px = x;
	return view;
}

Camera CircleUI::Move(const double x, const double y, const Camera &view)
{
	if (!_activated) {
		return view;
	}
	if (x == _px) {
		return view;
	}

	_cx = x;
	_cy = y;

	// A full drag (one end of the screen to the other) represents a semicircle
	float angle = static_cast<float>(_cx - _px) / screen_width * glm::pi<float>();

	// rotate around _up axis
	glm::mat4 rot(1.0f);
	rot = glm::rotate(rot, -angle, _up);
	glm::vec4 view_dir = glm::vec4(view.GetPosition(), 1) - glm::vec4(_center, 1);
	glm::vec4 _location = rot * view_dir + glm::vec4(_center, 1);

	// Assign camera new extrinsic
	glm::vec3 location(_location);
	glm::vec3 lookat = glm::normalize(_center - location);
	glm::vec3 right = glm::cross(lookat, _up);

	Camera result(Extrinsic(location, location + lookat, _up),
		view.GetIntrinsic());

	_px = _cx;
	_py = _cy;

	return result;
}

std::string CircleUI::Name() const
{
	return "circle";
}