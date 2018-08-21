#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "camera/Extrinsic.hpp"

// Interpolate rotation and translation simultaneously. Rotations are 
// converted to quaternions and interpolated by SLERP. Translation are 
// interpolated by LERP.
Extrinsic Interp(const Extrinsic& left, const Extrinsic &right, const float t = 0.f);



#endif