#ifndef CAMERA_H
#define CAMERA_H

#include <fstream>
#include <cmath>
#include "glm/glm.hpp"

class CamPose
{
private:
	glm::vec3 lookat;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 offset;

public:
	void Read(std::ifstream &);
	glm::vec3 GetLookat(void);
	glm::vec3 GetUp(void);
	glm::vec3 GetOffset(void);
};

class CamIntrinsic
{
private:
	float r[3][3];

public:
	void Read(std::ifstream &);
	glm::vec2 GetFoV(void);
	inline float GetCx(void) { return r[0][2]; }
	inline float GetCy(void) { return r[1][2]; }
	inline float GetFx(void) { return r[0][0]; }
	inline float GetFy(void) { return r[1][1]; }
};

void CamPose::Read(std::ifstream &f)
{
	f >> lookat.x >> lookat.y >> lookat.z
		>> right.x >> right.y >> right.z
		>> up.x >> up.y >> up.z
		>> offset.x >> offset.y >> offset.z;
	up = -up;
}

glm::vec3 CamPose::GetLookat(void)
{
	return lookat;
}

glm::vec3 CamPose::GetUp(void)
{
	return up;
}

glm::vec3 CamPose::GetOffset(void)
{
	return offset;
}

void CamIntrinsic::Read(std::ifstream &f)
{
	int index;

	f >> index 
		>> r[0][0] >> r[0][1] >> r[0][2]
		>> r[1][0] >> r[1][1] >> r[1][2]
		>> r[2][0] >> r[2][1] >> r[2][2];
}

glm::vec2 CamIntrinsic::GetFoV(void)
{
	float FoV_W = 2 * atan2(r[0][2], r[0][0]);
	float FoV_H = 2 * atan2(r[1][2], r[1][1]);

	return glm::vec2(FoV_W, FoV_H);
}

#endif