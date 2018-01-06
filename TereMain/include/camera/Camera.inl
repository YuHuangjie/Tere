#include <glm/gtc/matrix_transform.hpp>
#include "camera/Camera.hpp"

inline Camera::Camera()
{}

inline Camera::Camera(const Extrinsic &extrinsic, const Intrinsic &intrinsic)
	: mExtrinsic(extrinsic), mIntrinsic(intrinsic)
{}

inline void Camera::SetExtrinsic(const Extrinsic &extrinsic)
{
	mExtrinsic = extrinsic;
}

inline void Camera::SetIntrinsic(const Intrinsic &intrinsic)
{
	mIntrinsic = intrinsic;
}

inline glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(mExtrinsic.GetPos(), mExtrinsic.GetPos() - mExtrinsic.GetDir(), 
		mExtrinsic.GetUp());
}

inline glm::mat4 Camera::GetProjectionMatrix(float glnear, float glfar) const
{
	float cx = static_cast<float>(mIntrinsic.GetCx());
	float cy = static_cast<float>(mIntrinsic.GetCy());
	float fx = static_cast<float>(mIntrinsic.GetFx());
	float fy = static_cast<float>(mIntrinsic.GetFy());
	float ImgW = static_cast<float>(mIntrinsic.GetWidth());
	float ImgH = static_cast<float>(mIntrinsic.GetHeight());

	float l = -cx / fx * glnear;
	float r = (ImgW - cx) / fx * glnear;
	float b = (cy - ImgH) / fy * glnear;
	float t = cy / fy * glnear;

	glm::mat4 P(0);
	P[0][0] = 2 * glnear / (r - l);
	P[2][0] = (r + l) / (r - l);
	P[1][1] = 2 * glnear / (t - b);
	P[2][1] = (t + b) / (t - b);
	P[2][2] = -(glfar + glnear) / (glfar - glnear);
	P[2][3] = -1;
	P[3][2] = -2 * glnear * glfar / (glfar - glnear);

	return P;
}