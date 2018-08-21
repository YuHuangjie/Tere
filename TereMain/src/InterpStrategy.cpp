#include <algorithm>
#include <numeric>

#include "InterpStrategy.h"
#include "WeightedCamera.h"
#include "camera/Camera.hpp"

vector<WeightedCamera> DefaultInterpStrgFunc(
	const vector<Camera>& refCams,
	const Camera & virCam, const glm::vec3 & p, 
	const size_t maxn)
{
	const size_t nInterp = std::min<size_t>(refCams.size(), maxn);
	if (nInterp == 0) {
		return vector<WeightedCamera>();
	}

	vector<float> distances;
	
	// calculate cosine distances of virtual camera w.r.t. every
	// reference camera
	const glm::vec3 &virPos = virCam.GetPosition();
	const glm::vec3 &p2vir = glm::normalize(virPos - p);
	for (auto c : refCams) {
		const glm::vec3 &refPos = c.GetPosition();
		const float dis = glm::dot(p2vir, glm::normalize(refPos - p));
		distances.push_back(dis);
	}

	vector<size_t> indices(refCams.size());
	std::iota(indices.begin(), indices.end(), 0);

	// partial sort reference cameras and get nInterp cameras of 
	// least distances
	std::partial_sort(indices.begin(), indices.begin() + nInterp,
		indices.end(), [&distances](const size_t &a, const size_t &b) {
		return distances[a] > distances[b];
	});

	vector<float> weights;
	
	// assign weights according to cosine distances
	for (auto it = indices.cbegin(); it != indices.cbegin() + nInterp; ++it) {
		const float dist = distances[*it];
		const float weight = std::pow(dist, 4);
		weights.push_back(weight);
	}
	const float sumW = std::accumulate(weights.cbegin(), weights.cend(), 0.f);
	std::for_each(weights.begin(), weights.end(),
		[sumW](float &w) { w = w / sumW; });

	vector<WeightedCamera> interpCams;

	// construct weighted camera
	for (size_t i = 0; i < nInterp; ++i) {
		interpCams.push_back(WeightedCamera(static_cast<int>(indices[i]), weights[i]));
	}

	return interpCams;
}
