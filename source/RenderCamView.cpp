#include "RenderCamView.h"
#include <iostream>

void RenderCamView::setParameter(const glm::vec3 &_location, const glm::vec3 &_look, const glm::vec3 &_up)
{
	setParameter(_location, _look, _up, FovW, FovH, cx, cy, ImgW, ImgH);
}

void RenderCamView::setParameter(const glm::vec3 &_location, const glm::vec3 &_look, const glm::vec3 &_up,
	float _FovW, float _FovH, float _cx, float _cy, float _ImgW, float _ImgH)
{
	location = _location;
	lookat = glm::normalize(_look);
	up = glm::normalize(_up);
	FovH = _FovH;
	FovW = _FovW;
	aspect_ratio = FovW / FovH;
	cx = _cx;
	cy = _cy;
	fx = cx / std::tan(FovW / 2);
	fy = cy / std::tan(FovH / 2);
	ImgW = _ImgW;
	ImgH = _ImgH;
}

glm::mat4 RenderCamView::GetViewMatrix(float lambda) const
{
	glm::vec3 location = glm::vec3(this->location[0] * lambda,
		this->location[1] * lambda, this->location[2] * lambda);
	return glm::lookAt(location, location + lookat, up);
}

glm::mat4 RenderCamView::GetProjectionMatrix(float glnear, float glfar) const
{
	//glm::mat4 res = glm::perspective(FovH, aspect_ratio, glnear, glfar);
	//return res;

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
