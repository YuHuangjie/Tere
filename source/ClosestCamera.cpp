#include "ClosestCamera.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

ClosestCameraSet::ClosestCameraSet(const int N)
	: cam_nr(N)
{
	SetFaultCameras();
	last_camera_id = glm::ivec4(-1, -1, -1, -1);
}

void ClosestCameraSet::SetFaultCameras(void)
{
	this->camera_id = glm::ivec4(-1, -1, -1, -1);
	this->weights = glm::vec4(0, 0, 0, 0);
}

void ClosestCameraSet::SetCameras(const glm::ivec4 &cameras, const glm::vec4 &weights)
{
	camera_id = cameras;
	this->weights = weights;
	mask = glm::ivec4(0, 0, 0, 0);

	// relocate camera ids so that same camera id in new and old camera_set are put at 
	// the same locaiton
	for (int i = 0; i != 4; ++i) {
		for (int j = 0; j != 4; ++j) {
			if (camera_id[i] == last_camera_id[j] && mask[i] == 0) {
				std::swap(camera_id[i], camera_id[j]);
				std::swap(this->weights[i], this->weights[j]);
				mask[j] = 1;
				if (i != j) {
					i = -1;
					break;
				}
			}
		}
	}

	last_camera_id = camera_id;
}

void ClosestCameraSet::SetCamerasNoOpt(const glm::ivec4 &cameras, const glm::vec4 &weights)
{
	camera_id = cameras;
	this->weights = weights;
}