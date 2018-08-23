#include <cstdlib>
#include <stdexcept>
#include <string>

#include "MultiLinearUI.h"
#include "camera/Camera.hpp"
#include "Interpolation.h"

using std::vector;
using std::runtime_error;

extern float NormalizePoint(const float p, const float list);

enum Direction : int
{
	NEGTIVE = -1,
	POSITIVE = 1
};

enum Major : unsigned int
{
	ROTATE_ALONG_ROW,
	ROTATE_ALONG_COLUMN
};

MultiLinearUI::MultiLinearUI(const vector<Camera> &camList,
	const size_t rows, const int p) 
	: UserInterface(),
	_camList(camList),
	_nCams(camList.size()),
	_rows(0),
	_cols(0),
	_activated(false),
	_px(0.0),
	_py(0.0),
	_cx(0.0),
	_cy(0.0),
	_pRow(0.f),
	_pCol(0.f),
	_direction(NEGTIVE),
	_rowReversed(false),
	_major(ROTATE_ALONG_ROW),
	_nearest(0)
{
	if (_nCams == 0) {
		throw runtime_error("MultiLinearUI: empty list");
	}

	// calculate rows and columns
	std::div_t res = std::div(static_cast<int>(_nCams), static_cast<int>(rows));
	if (res.rem != 0) {
		throw runtime_error("MultiLinearUI: incorrect arguments");
	}
	_rows = rows;
	_cols = res.quot;

	// where in the layout is the initial pointer
	_pRow = static_cast<float>(p / _cols);
	_pCol = static_cast<float>(p % _cols);
	_nearest = p;

	if (_cols >= 2) {
		// reverse list if the sequence of cameras doesn't match the direction
		// in the diagram
		glm::vec4 cam2InCam1 = _camList[0].GetViewMatrix() * glm::vec4(
			_camList[1].GetPosition(), 1.0f);
		if (cam2InCam1.x < 0.f) {
			_rowReversed = true;
		}
	}

	SetResolution(0, 0);
}

std::string MultiLinearUI::Name() const
{
	return "multilinear";
}

void MultiLinearUI::Touch(const double x, const double y)
{
	_activated = true;
	_majorLock = false;
	_px = x;
	_py = y;
}

Camera MultiLinearUI::Leave(const double x, const double y, const Camera &view)
{
	_activated = false;
	_px = x;
	_py = y;

	if (_major == ROTATE_ALONG_COLUMN && _direction == NEGTIVE) {
		int left = static_cast<int>(std::floor(_pCol));
		if (left == -1) { left = _cols - 1; }
		_nearest = _pRow * _cols + left;
		_pCol = left;
	}
	else if (_major == ROTATE_ALONG_COLUMN && _direction == POSITIVE) {
		int right = static_cast<int>(std::ceil(_pCol));
		_nearest = _pRow * _cols + right;
		_pCol = right;
	}
	else if (_major == ROTATE_ALONG_ROW && _direction == NEGTIVE) {
		int left = static_cast<int>(std::floor(_pRow));
		_nearest = left * _cols + _pCol;
		_pRow = left;
	}
	else {
		int right = static_cast<int>(std::ceil(_pRow));
		_nearest = right * _cols + _pCol;
		_pRow = right;
	}

	return _camList[_nearest];
}

Camera MultiLinearUI::Move(const double x, const double y, const Camera &view)
{
	if (!_activated) {
		return view;
	}
	if (x == _px && y == _py) {
		return view;
	}

	// determine the major rotation axis
	if (!_majorLock) {
		_major = ROTATE_ALONG_ROW;
		if (std::abs(x - _px) > std::abs(y - _py)) {
			_major = ROTATE_ALONG_COLUMN;
		}
		_majorLock = true;
	}

	if (_major == ROTATE_ALONG_COLUMN) {
		_cx = x;
		_direction = _cx > _px ? NEGTIVE : POSITIVE;
		_direction = static_cast<Direction>(_direction * (_rowReversed ? -1 : 1));

		// a full drag (from one end of the screen to the other) covers half list
		float aFullDrag = _cols / 2.f;
		float cover = (_cx - _px) / screen_width * aFullDrag;

		_pCol += cover * (_rowReversed ? 1 : -1);
		_pCol = NormalizePoint(_pCol, _cols);

		// find left and right reference camera
		int left = static_cast<int>(std::floor(_pCol));
		int right = static_cast<int>(std::ceil(_pCol));
		float t = _pCol - left;

		// handle singularity
		if (left == right) {
			if (left == -1 || left == _cols - 1) {
				right = 0;
			}
			else {
				right = left + 1;
			}
		}
		if (left == -1) { left = _cols - 1; }

		// interpolate pose
		Extrinsic interp = Interp(_camList[_pRow*_cols + left].GetExtrinsic(),
			_camList[_pRow*_cols + right].GetExtrinsic(), t);
		Camera result(interp, view.GetIntrinsic());

		_px = _cx;
		return result;
	}
	else /* _major==ROTATE_ALONG_ROW */ {
		_cy = y;
		_direction = _cy > _py ? NEGTIVE : POSITIVE;

		float aFullDrag = _rows;
		float cover = (_cy - _py) / screen_height * aFullDrag;

		_pRow -= cover;
		if (_pRow < 0.f) { _pRow = 0.f; }
		else if (_pRow > _rows - 1) { _pRow = _rows - 1; }

		// find left and right reference camera
		int left = static_cast<int>(std::floor(_pRow));
		int right = static_cast<int>(std::ceil(_pRow));
		float t = _pRow - left;

		if (left == right) { 
			return _camList[left * _cols + _pCol];
		}

		// interpolate pose
		Extrinsic interp = Interp(_camList[left*_cols + _pCol].GetExtrinsic(),
			_camList[right*_cols + _pCol].GetExtrinsic(), t);
		Camera result(interp, view.GetIntrinsic());

		_py = _cy;
		return result;
	}
}

size_t MultiLinearUI::GetNearestRef() const
{
	return _nearest;
}

//float NormalizePoint(const float p, const float list)
//{
//	float result = p;
//
//	// Clamp p to (-1, list-1). Here, -1 represents the last camera
//	while (result > list - 1) {
//		result -= list;
//	}
//	while (result < -1) {
//		result += list;
//	}
//
//	return result;
//}