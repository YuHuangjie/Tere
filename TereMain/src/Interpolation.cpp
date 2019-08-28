#include <algorithm>
#include "glm/gtx/quaternion.hpp"

#include "Interpolation.h"

Extrinsic Interp(const Extrinsic & left, const Extrinsic & right, const float t)
{
	float _t = std::min<float>(1.f, std::max<float>(0.f, t));

	// convert left rotation to quaternion
	glm::mat3 rotLeft(left.Right(), left.Up(), left.Dir());
	glm::vec3 transLeft = left.Pos();
	glm::quat quatLeft = glm::toQuat(rotLeft);

	// convert right rotation to quaternion
	glm::mat3 rotRight(right.Right(), right.Up(), right.Dir());
	glm::vec3 transRight = right.Pos();
	glm::quat quatRight = glm::toQuat(rotRight);

	// SLERP on rotation component
	glm::quat mixQuat = glm::slerp(quatLeft, quatRight, t);
	glm::mat3 mixRot = glm::toMat3(mixQuat);

	// LERP on translation component
	glm::vec3 mixTrans = transLeft + (transRight - transLeft) * t;

	return Extrinsic(&mixTrans[0], &(mixTrans - mixRot[2])[0], &(mixRot[1])[0]);
}
