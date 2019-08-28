#ifndef CAMERA_CAMERA_H
#define CAMERA_CAMERA_H

#include "camera/Intrinsic.hpp"
#include "camera/Extrinsic.hpp"

struct Camera
{
	Intrinsic intrin;
	Extrinsic extrin;
};

#endif /* CAMERA_CAMERA_H */
