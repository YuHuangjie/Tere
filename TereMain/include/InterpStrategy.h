#ifndef INTERPSTRATEGY_H
#define INTERPSTRATEGY_H

#include <vector>
#include <cstddef>
#include <glm/glm.hpp>

// forward declarations
class WeightedCamera;
class Camera;


using std::vector;
using std::size_t;

vector<WeightedCamera> DefaultInterpStrgFunc(
	const vector<Camera> &refCams,
	const Camera &virCam, const glm::vec3 &p,
	const size_t maxn);

#endif