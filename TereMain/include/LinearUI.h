#ifndef LINEARUI_H
#define LINEARUI_H

#include <vector>

#include "UserInterface.h"

using std::vector;

enum Direction : int;
enum Major : unsigned int;

// User interface for multi-row image sequences
//
class LinearUI : public UserInterface
{
public:
	// @params rows	rows of ref cameras
	// @params P	the position of current virtual camera among the 
	//		camera list. The following disgram shows a camera list with 3 rows.
	//      And a virtual camera being at second rows third column when p=n+2
	//		P must be consistent with the initial pose of virtual camera to 
	//		render rightly.
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
	LinearUI(const vector<Camera> &camList, const size_t rows,
		const int p);

	virtual std::string Name() const override;
	virtual void Touch(const double x, const double y) override;
	virtual Camera Leave(const double x, const double y, const Camera &view) override;
	virtual Camera Move(const double x, const double y, const Camera &view) override;

	// hint interpolation cameras
	virtual vector<size_t> HintInterp() override;

protected:
	const vector<Camera>& _camList;
	const size_t _nCams;
	size_t _rows;
	size_t _cols;

	bool _activated;
	double _px, _py;
	double _cx, _cy;
	float _pRow;		// row pointer to the layout
	float _pCol;		// col pointer to the layout
	Direction _direction;
	bool _rowReversed;
	Major _major;
	bool _majorLock;	// major shouldn't change through interaction

	vector<size_t> _neighbors;
};


#endif 