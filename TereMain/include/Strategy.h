#ifndef INTERPSTRATEGY_H
#define INTERPSTRATEGY_H

#include <vector>
#include <cstddef>
#include <glm/glm.hpp>

// forward declarations
class Camera;

using std::vector;
using std::size_t;

vector<size_t> DefaultIndexStrgFunc (const vector<Camera> &ref,
	const Camera &vir, const glm::vec3 &p, const size_t maxn);

vector<float> DefaultWeightStrgFunc(const vector<Camera> &ref,
	const Camera &vir, const glm::vec3 &p, const vector<size_t> &indices);

#endif