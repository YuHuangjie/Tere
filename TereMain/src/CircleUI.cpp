#include "CircleUI.h"

CircleUI::CircleUI(const int sw, const glm::vec3 & up, const glm::vec3 & center)
	: UserInterface(),
	_up(up),
	_center(center),
	_activated(false),
	_px(0.0),
	_py(0.0),
	_cx(0.0),
	_cy(0.0)
{
	SetResolution(sw, 0);
}

void CircleUI::Touch(const double x, const double y)
{
	_activated = true;
	_px = x;
}

void CircleUI::Leave(const double x, const double y)
{
	_activated = false;
	_px = x;
}

void CircleUI::Move(const double x, const double y, Camera &view)
{
	if (!_activated) {
		return;
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
	view.SetExtrinsic(Extrinsic(location, location + lookat, _up));

	_px = _cx;
	_py = _cy;
}