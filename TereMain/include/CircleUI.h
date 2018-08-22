#ifndef CIRCLEUI_H
#define CIRCLEUI_H

#include "glm/glm.hpp"
#include "UserInterface.h"

class CircleUI : public UserInterface
{
public:
	CircleUI(const int sw, const glm::vec3 &up, const glm::vec3 &center);

	virtual std::string Name() const override;
	virtual void Touch(const double x, const double y) override;
	virtual Camera Leave(const double x, const double y, const Camera &view) override;
	virtual Camera Move(const double x, const double y, const Camera &view) override;

protected:
	glm::vec3 _up;
	glm::vec3 _center;

	bool _activated;
	double _px, _py;
	double _cx, _cy;
};

#endif