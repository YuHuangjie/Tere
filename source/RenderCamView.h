#ifndef RENDERCAMVIEW_H
#define RENDERCAMVIEW_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class RenderCamView
{
public:
	RenderCamView() {};

	void setParameter(const glm::vec3 &_location, const glm::vec3 &_look, const glm::vec3 &_up);
	void setParameter(const glm::vec3 &_location, const glm::vec3 &_look, const glm::vec3 &_up,
		float _FovW, float _FovH, float cx, float cy, float ImgW, float ImgH);

	glm::mat4 GetViewMatrix(float lambda = 1) const;
	glm::mat4 GetProjectionMatrix(float near, float far) const;

	inline glm::vec3 GetRight(void)  const{ return glm::cross(lookat, up); }
	inline glm::vec3 GetPosition(void)  const{ return location; }
	inline glm::vec3 GetLookAt(void)  const{ return lookat; }
	inline glm::vec3 GetUp(void)  const{ return up; }
	inline float GetFOVH(void)  const{ return FovH; }
	inline float GetFOVW(void)  const{ return FovW; }
	inline float GetCx(void)  const{ return cx; }
	inline float GetCy(void)  const{ return cy; }
	inline float GetFx(void)  const{ return fx; }
	inline float GetFy(void)  const{ return fy; }
	inline float GetWidth(void)  const{ return ImgW; }
	inline float GetHeight(void)  const{ return ImgH; }

private:
	glm::vec3 location;
	glm::vec3 lookat;
	glm::vec3 up;

	float FovW;
	float FovH;
	float aspect_ratio;
	float cx, cy;
	float fx, fy;
	float ImgW, ImgH;
};

#endif