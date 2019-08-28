#ifndef WEIGHTEDCAMERA_H
#define WEIGHTEDCAMERA_H

struct WeightedCamera
{
	WeightedCamera(int i, float w) : index(i), weight(w) {}

	int index;
	float weight;
};

#endif