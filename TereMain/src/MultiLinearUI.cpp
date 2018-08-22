#include <cstdlib>
#include <stdexcept>
#include <string>

#include "MultiLinearUI.h"
#include "camera/Camera.hpp"
#include "Interpolation.h"

using std::vector;
using std::runtime_error;

enum Direction : unsigned int
{
	NEGTIVE,
	POSITIVE
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

	SetResolution(0, 0);
}

std::string MultiLinearUI::Name() const
{
	return "multilinear";
}

void MultiLinearUI::Touch(const double x, const double y)
{
	_activated = true;
	_px = x;
	_py = y;
}

Camera MultiLinearUI::Leave(const double x, const double y, const Camera &view)
{
	_activated = false;
	_px = x;
	_py = y;

	return view;
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
	_major = ROTATE_ALONG_ROW;
	if (std::abs(x - _px) > std::abs(y - _py)) {
		_major = ROTATE_ALONG_COLUMN;
	}

	if (_major == ROTATE_ALONG_COLUMN) {
		_cx = x;
		_direction = _cx > _px ? NEGTIVE : POSITIVE;

		// a full drag (from one end of the screen to the other) covers half list
		float aFullDrag = _cols / 2.f;
		float cover = (_cx - _px) / screen_width * aFullDrag;

		_pRow += cover;
		_pRow = NormalizePoint(_pRow, _cols);

		// find left and right reference camera
		int left = static_cast<int>(std::floor(_pRow));
		int right = static_cast<int>(std::ceil(_pRow));
		float t = _pRow - left;

		// handle singularity
		if (left == right) {
			return _camList[_pRow*_cols + left];
		}

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

		float aFullDrag = _rows / 2.f;
		float cover = (_cy - _py) / screen_height * aFullDrag;

		_pCol += cover;
		if (_pCol < 0.f) { _pCol = 0.f; }
		else if (_pCol > _rows - 1) { _pCol = _rows - 1; }

		// find left and right reference camera
		int left = static_cast<int>(std::floor(_pCol));
		int right = static_cast<int>(std::ceil(_pCol));
		float t = _pCol - left;

		if (left == right) { 
			return _camList[left * _cols + _pRow];
		}

		// interpolate pose
		Extrinsic interp = Interp(_camList[left*_cols + _pRow].GetExtrinsic(),
			_camList[right*_cols + _pRow].GetExtrinsic(), t);
		Camera result(interp, view.GetIntrinsic());

		_px = _cx;
		return result;
	}
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