#ifndef ARCBALLUI_H
#define ARCBALLUI_H

#include "glm/fwd.hpp"
#include "UserInterface.h"

class ArcballUI : public UserInterface
{
public:
	ArcballUI(glm::vec3 last_up, glm::vec3 look_center);

	virtual std::string Name() const override;
	virtual void Touch(const double x, const double y) override;
	virtual Camera Leave(const double x, const double y, const Camera &view) override;
	virtual Camera Move(const double x, const double y, const Camera &view) override;

private:
	glm::vec3 GetArcballVector(double x, double y, const Camera &view);

	glm::vec3 look_center;
	glm::vec3 last_up;

	// UI
	bool arcball_on = false;
	double last_mx, last_my;
	double cur_mx, cur_my;
};

#endif