#ifndef INTRINSIC_HPP
#define INTRINSIC_HPP

#include <cmath>
#include <glm/glm.hpp>

struct Intrinsic
{
	/* Center of projection */
	float cx, cy;

	/* Focal length */
	float fx, fy;

	Intrinsic()	: cx(0.0), cy(0.0), fx(0.0), fy(0.0)
	{}

	Intrinsic(const float cx, const float cy, const float fx, const float fy)
		: cx(cx), cy(cy), fx(fx), fy(fy)
	{}

	Intrinsic(const float K[9]) 
		: cx(K[2]), cy(K[5]), fx(K[0]), fy(K[4])
	{}

	inline float fovw() const { return 2 * std::atan2(cx, fx); }
	inline float fovh() const { return 2 * std::atan2(cy, fy); }

	/* Get projection matrix (column major) */
	inline glm::mat4 ProjMat(const float near, const float far,
		const float w = 0, const float h = 0) const
	{
		float width = (w == 0 ? 2 * cx : w);
		float height = (h == 0 ? 2 * cy : h);
		float l = -cx / fx * near;
		float r = (width - cx) / fx * near;
		float b = (cy - height) / fy * near;
		float t = cy / fy * near;

		glm::mat4 P(0);
		P[0][0] = 2 * near / (r - l);
		P[2][0] = (r + l) / (r - l);
		P[1][1] = 2 * near / (t - b);
		P[2][1] = (t + b) / (t - b);
		P[2][2] = -(far + near) / (far - near);
		P[2][3] = -1;
		P[3][2] = -2 * near * far / (far - near);

		return P;
	}
};

#endif
