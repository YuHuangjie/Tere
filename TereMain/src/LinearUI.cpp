#include <cstdlib>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "LinearUI.h"
#include "camera/Extrinsic.hpp"
#include "Interpolation.h"
#include "Error.h"

using std::vector;

static float NormalizePoint(const float p, const float list);

LinearUI::LinearUI(const vector<Extrinsic> &list,
	const size_t rows, const int p, const float width, const float height)
	: UserInterface(),
	_extrinsics(list),
	_nCams(list.size()),
	_rows(0),
	_cols(0),
	_singleRow(false),
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
	_neighbors(),
	_width(width),
	_height(height)
{
	if (_nCams == 0) {
		THROW_ON_ERROR("LinearUI: empty list");
	}

	// calculate rows and columns
	std::div_t res = std::div(static_cast<int>(_nCams), static_cast<int>(rows));
	if (res.rem != 0) {
		THROW_ON_ERROR("LinearUI: incorrect arguments");
	}
	_rows = rows;
	_cols = res.quot;
	_singleRow = (_rows == 1);
	_majorLock = true;
	_major = ROTATE_ALONG_COLUMN;

	// where in the layout is *p*
	_pRow = static_cast<float>(p / _cols);
	_pCol = static_cast<float>(p % _cols);
	_neighbors.push_back(_pRow * _cols + _pCol);

	if (_cols >= 2) {
		// The second camera is on the left of the first camera.
		// In this case, some calculation should change.
		glm::mat4 world2FirstCam = _extrinsics[0].viewMat;
		glm::vec4 posOfSecondCam = glm::vec4(_extrinsics[1].Pos(), 1.f);
		glm::vec4 cam2InCam1 = world2FirstCam * posOfSecondCam;
		if (cam2InCam1.x < 0.f) {
			_rowReversed = true;
		}
	}

	SetResolution(0, 0);
}

std::string LinearUI::Name() const
{
	return "linear";
}

void LinearUI::SetResolution(const float width, const float height)
{
	_width = width;
	_height = height;
}

void LinearUI::Touch(const float x, const float y)
{
	_activated = true;
	_majorLock = false;
	_px = x;
	_py = y;
}

Extrinsic LinearUI::Leave(const float x, const float y, const Extrinsic &view)
{
	_activated = false;
	_px = x;
	_py = y;
	size_t nearest = 0;

	if (_major == ROTATE_ALONG_COLUMN && _direction == NEGTIVE) {
		int left = static_cast<int>(std::floor(_pCol));
		if (left == -1) { left = _cols - 1; }
		nearest = _pRow * _cols + left;
		_pCol = left;
	}
	else if (_major == ROTATE_ALONG_COLUMN && _direction == POSITIVE) {
		int right = static_cast<int>(std::ceil(_pCol));
		nearest = _pRow * _cols + right;
		_pCol = right;
	}
	else if (_major == ROTATE_ALONG_ROW && _direction == NEGTIVE) {
		int left = static_cast<int>(std::floor(_pRow));
		nearest = left * _cols + _pCol;
		_pRow = left;
	}
	else {
		int right = static_cast<int>(std::ceil(_pRow));
		nearest = right * _cols + _pCol;
		_pRow = right;
	}

	return _extrinsics[nearest];
}

Extrinsic LinearUI::Move(const float x, const float y, const Extrinsic &view)
{
	if (!_activated) {
		return view;
	}
	if (x == _px && y == _py) {
		return view;
	}

	// determine the major rotation axis
	if (!_singleRow && !_majorLock) {
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
		float cover = (_cx - _px) / _width * aFullDrag;

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
		size_t neighbor1 = _pRow*_cols + left;
		size_t neighbor2 = _pRow*_cols + right;
		Extrinsic interp = Interp(_extrinsics[neighbor1],
			_extrinsics[neighbor2], t);
		
		// interpolation hints
		_neighbors.clear();
		_neighbors.push_back(neighbor1);
		_neighbors.push_back(neighbor2);

		_px = _cx;
		return interp;
	}
	else /* _major==ROTATE_ALONG_ROW */ {
		_cy = y;
		_direction = _cy > _py ? NEGTIVE : POSITIVE;

		float aFullDrag = _rows;
		float cover = (_cy - _py) / _height * aFullDrag;

		_pRow -= cover;
		if (_pRow < 0.f) { _pRow = 0.f; }
		else if (_pRow > _rows - 1) { _pRow = _rows - 1; }

		// find left and right reference camera
		int left = static_cast<int>(std::floor(_pRow));
		int right = static_cast<int>(std::ceil(_pRow));
		float t = _pRow - left;

		// interpolate pose
		size_t neighbor1 = left*_cols + _pCol;
		size_t neighbor2 = right*_cols + _pCol;
		Extrinsic interp = Interp(_extrinsics[neighbor1],
			_extrinsics[neighbor2], t);

		// interpolation hints
		_neighbors.clear();
		_neighbors.push_back(neighbor1);
		_neighbors.push_back(neighbor2);

		_py = _cy;
		return interp;
	}
}

vector<size_t> LinearUI::HintInterp()
{
	return _neighbors;
}

static float NormalizePoint(const float p, const float list)
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