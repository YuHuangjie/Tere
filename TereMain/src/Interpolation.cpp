#include <algorithm>
#include <glm/gtx/quaternion.hpp>

#include "Interpolation.h"

Extrinsic Interp(const Extrinsic & left, const Extrinsic & right, const float t)
{
	float _t = std::min<float>(1.f, std::max<float>(0.f, t));

	// convert left rotation to quaternion
	glm::mat3 rotLeft(left.GetRight(), -left.GetUp(), -left.GetDir());
	glm::vec3 transLeft = left.GetPos();
	glm::quat quatLeft = glm::toQuat(rotLeft);

	// convert right rotation to quaternion
	glm::mat3 rotRight(right.GetRight(), -right.GetUp(), -right.GetDir());
	glm::vec3 transRight = right.GetPos();
	glm::quat quatRight = glm::toQuat(rotRight);

	// SLERP on rotation component
	glm::quat mixQuat = glm::slerp(quatLeft, quatRight, t);
	glm::mat3 mixRot = glm::toMat3(mixQuat);

	// LERP on translation component
	glm::vec3 mixTrans = transLeft + (transRight - transLeft) * t;

	return Extrinsic(mixTrans, mixTrans + mixRot[2], -mixRot[1]);
}
