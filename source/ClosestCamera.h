#ifndef CLOSESTCAMERA_H
#define CLOSESTCAMERA_H

#include "glm/glm.hpp"

class ClosestCameraSet
{
public:
	ClosestCameraSet(const int N);

	void SetFaultCameras(void);
	void SetCameras(const glm::ivec4 &cameras, const glm::vec4 &weights);
	void SetCamerasNoOpt(const glm::ivec4 &cameras, const glm::vec4 &weights);

	inline int GetCamNr(void) const { return cam_nr; }
	inline glm::ivec4 GetCamera(void) const { return camera_id; }
	inline int GetCamera(int index) const { return camera_id[index]; }
	inline glm::vec4 GetWeight(void) const { return weights; }
	inline bool IsModified(int index) const { return (mask[index] == 1 ? false : true); }

private:
	int cam_nr;

	glm::ivec4 mask;		// 0: camera modified, 1: camera unmodified
	glm::ivec4 last_camera_id;
	glm::ivec4 camera_id;
	glm::vec4 weights;
};

#endif