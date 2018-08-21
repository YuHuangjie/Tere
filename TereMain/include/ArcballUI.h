#ifndef ARCBALLUI_H
#define ARCBALLUI_H

#include "glm/glm.hpp"
#include "UserInterface.h"

class ArcballUI : public UserInterface
{
public:
	ArcballUI(const int sw, const int sh, glm::vec3 last_up, 
		glm::vec3 look_center);

	virtual void Touch(const double x, const double y) override;
	virtual void Leave(const double x, const double y) override;
	virtual void Move(const double x, const double y, Camera &view) override;

private:
	glm::vec3 GetArcballVector(double x, double y, Camera view);

	glm::vec3 look_center;
	glm::vec3 last_up;

	// UI
	bool arcball_on = false;
	double last_mx, last_my;
	double cur_mx, cur_my;
};

#endif