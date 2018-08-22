#ifndef LINEARUI_H
#define LINEARUI_H

#include <vector>

#include "UserInterface.h"

using std::vector;

class LinearUI : public UserInterface
{
public:
	// P represents the position of current virtual camera among the 
	// camera list. The following diagram depicts a virtual camera being 
	// the same as the 3rd reference camera when p=2.
	// P must be consistent with the initial pose of virtual camera to 
	// render rightly. Therefore, p is normally an integer.
	// 
	// |___|___|___|......|___|
	// 0   1   2   3  .. n-1  n   
	//         ^
	//         p
	//
	LinearUI(const vector<Camera> &camList, const int p);

	virtual std::string Name() const override;
	virtual void Touch(const double x, const double y) override;
	virtual Camera Leave(const double x, const double y, const Camera &view) override;
	virtual Camera Move(const double x, const double y, const Camera &view) override;
	size_t GetNearestRef() const;

protected:
	const vector<Camera>& _camList;
	const size_t _nCams;

	bool _activated;
	double _px, _py;
	double _cx, _cy;
	float _point;		// point int the list
	int _direction;		// sliding direction
	size_t _nearest;	// nearest reference camera
};

float NormalizePoint(const float p, const float list);

#endif 