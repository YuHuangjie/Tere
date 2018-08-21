#ifndef WEIGHTEDCAMERA_H
#define WEIGHTEDCAMERA_H

class WeightedCamera
{
public:
	WeightedCamera(int i, float w) : index(i), weight(w) {}

	int index;
	float weight;
};

#endif