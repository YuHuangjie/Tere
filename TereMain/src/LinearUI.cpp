#include "LinearUI.h"
#include "Interpolation.h"
#include "camera/Camera.hpp"

LinearUI::LinearUI(const vector<Camera> &camList, const int p)
	: UserInterface(),
	_camList(camList),
	_nCams(_camList.size()),
	_activated(false),
	_px(0.0),
	_py(0.0),
	_cx(0.0),
	_cy(0.0),
	_point(p),
	_direction(1),
	_nearest(0)
{
	SetResolution(0, 0);
}

void LinearUI::Touch(const double x, const double y)
{
	_activated = true;
	_px = x;
}

Camera LinearUI::Leave(const double x, const double y, const Camera &view)
{
	_activated = false;
	_px = x;
	
	if (_direction == 1) {
		int left = static_cast<int>(std::floor(_point));
		if (left == -1) { left = _nCams - 1; }
		_nearest = left;
		return _camList[left];
	}
	else {
		int right = static_cast<int>(std::ceil(_point));
		_nearest = right;
		return _camList[right];
	}
}

Camera LinearUI::Move(const double x, const double y, const Camera &view)
{
	if (!_activated) {
		return view;
	}
	if (x == _px) {
		return view;
	}

	_cx = x;
	_direction = _cx > _px ? -1 : 1;

	// a full drag (from one end of the screen to the other) covers half list
	float aFullDragCover = _camList.size() / 2.f;
	float cover = (_cx - _px) / screen_width * aFullDragCover;
	
	_point += cover;
	_point = NormalizePoint(_point, _nCams);
	
	// find left and right reference camera
	int left = static_cast<int>(std::floor(_point));
	int right = static_cast<int>(std::ceil(_point));
	float t = _point - left;

	// handle singularity
	if (left == right) {
		if (left == -1 || left == _nCams - 1) {
			right = 0;
		}
		else {
			right = left + 1;
		}
	}
	if (left == -1) { left = _nCams - 1; }

	// interpolate pose
	Extrinsic interp = Interp(_camList[left].GetExtrinsic(),
		_camList[right].GetExtrinsic(), t);
	Camera result(interp, view.GetIntrinsic());

	_px = _cx;

	return result;
}

std::string LinearUI::Name() const
{
	return "linear";
}

size_t LinearUI::GetNearestRef() const
{
	return _nearest;
}

float NormalizePoint(const float p, const float list)
{
	float result = p;

	// Clamp p to (-1, list-1). Here, -1 represents the last camera
	while (result > list - 1) {
		result -= list;
	}
	while (result < -1) {
		result += list;
	}

	return result;
}