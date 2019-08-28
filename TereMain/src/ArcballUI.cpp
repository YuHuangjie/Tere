#include <fstream>
#include <sstream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <list>

#include "ArcballUI.h"
#include "Arcball.h"

using namespace std;

ArcballUI::ArcballUI(const float width, const float height, const glm::vec3 &c)
	:_arcball(new ArcBall_t(width, height)),
	_rot(new Matrix3f_t),
	_center(c)
{}

std::string ArcballUI::Name() const
{
	return std::string("Arcball");
}

void ArcballUI::SetResolution(const float width, const float height)
{
	_arcball.reset(new ArcBall_t(width, height));
}

void ArcballUI::Touch(const float x, const float y)
{
	Point2fT mouse = { x, y };

	if (!_arcball->isDragging) {
		_arcball->isDragging = true;
		_arcball->click(&mouse);
	}
}

Extrinsic ArcballUI::Leave(const float x, const float y, const Extrinsic &view)
{
	_arcball->isDragging = false;
	*_rot = Matrix3f_t({ 1, 0, 0, 0, 1, 0, 0, 0, 1 });
	return view;
}

Extrinsic ArcballUI::Move(const float x, const float y, const Extrinsic &view)
{
	Point2fT mouse = { x, y };
	Extrinsic newView = view;

	if (_arcball->isDragging) {
		Quat4fT quat;

		// calculate dragging rotation
		_arcball->drag(&mouse, &quat);
		Matrix3fSetRotationFromQuat4f(_rot.get(), &quat);
		_arcball->click(&mouse);

		// apply rotation to view
		glm::mat3 rot(glm::vec3(_rot->s.M00, _rot->s.M10, _rot->s.M20),
			glm::vec3(_rot->s.M01, _rot->s.M11, _rot->s.M21),
			glm::vec3(_rot->s.M02, _rot->s.M12, _rot->s.M22));

		glm::mat3 newRot = rot * glm::mat3(view.viewMat);
		// TODO: Try to figure out why?
		glm::vec3 newPos = glm::transpose(newRot) * glm::mat3(view.viewMat) * (view.Pos() - _center) + _center;
		
		newView.viewMat = newRot;
		newView.viewMat[3] = glm::vec4(-newRot * newPos, 1.f);
	}

	return newView;
}

vector<size_t> ArcballUI::HintInterp()
{
	return vector<size_t>();
}
