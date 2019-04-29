#ifndef LINEARUI_H
#define LINEARUI_H

#include <vector>
#include "UserInterface.h"
#include "camera/Extrinsic.hpp"

using std::vector;

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

// User interface for multi-row image sequences
//
class LinearUI : public UserInterface
{
public:
	// @params rows	rows of ref cameras
	// @params P	the position of current virtual camera among the 
	//		camera list. The following diagram shows a camera list with 3 rows
	//      and a virtual camera being at second row third column when p=n+2.
	//		P must be consistent with the initial pose of virtual camera to 
	//		render correctly.
	//      When user drag leftward, p increases, unless _rowReversed is set.
	// 
	// |____|____|____|......|
	// 0    1    2    3  .. n-1 
	// |____|____|____|......|     
	// n   n+1  n+2  n+3    2n-1  
	//           ^
	//           p
	// |____|____|____|......|
	// 2n  2n+1 2n+2 2n+3   3n-1
	//
	LinearUI(const vector<Extrinsic> &list, const size_t rows,
		const int p, const float width, const float height);

	virtual std::string Name() const override;
	virtual void SetResolution(const float width, const float height) override;
	virtual void Touch(const float x, const float y) override;
	virtual Extrinsic Leave(const float x, const float y, const Extrinsic &cur) override;
	virtual Extrinsic Move(const float x, const float y, const Extrinsic &cur) override;

	// hint interpolation cameras
	virtual vector<size_t> HintInterp() override;

protected:
	vector<Extrinsic> _extrinsics;
	const size_t _nCams;
	size_t _rows;
	size_t _cols;
	bool _singleRow;

	bool _activated;
	float _px, _py;
	float _cx, _cy;
	float _pRow;		// row pointer to the layout
	float _pCol;		// col pointer to the layout
	Direction _direction;
	bool _rowReversed;
	Major _major;
	bool _majorLock;	// major shouldn't change through interaction

	vector<size_t> _neighbors;

	float _width, _height;
};


#endif 