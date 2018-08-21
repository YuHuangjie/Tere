#include <algorithm>
#include <glm/gtx/quaternion.hpp>

#include "Interpolation.h"

Extrinsic Interp(const Extrinsic & left, const Extrinsic & right, const float t)
{
	float _t = std::min<float>(1.f, std::max<float>(0.f, t));

	glm::mat3 rotLeft(left.GetRight(), -left.GetUp(), -left.GetDir());
	glm::vec3 transLeft = left.GetPos();
	glm::quat quatLeft = glm::toQuat(rotLeft);

	glm::mat3 rotRight(right.GetRight(), -right.GetUp(), -right.GetDir());
	glm::vec3 transRight = right.GetPos();
	glm::quat quatRight = glm::toQuat(rotRight);

	return Extrinsic();
}
