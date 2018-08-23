#include <algorithm>
#include <numeric>

#include "Strategy.h"
#include "WeightedCamera.h"
#include "camera/Camera.hpp"


vector<size_t> DefaultIndexStrgFunc(const vector<Camera> &refCams,
	const Camera &virCam, const glm::vec3 &p, const size_t maxn)
{
	const size_t nInterp = std::min<size_t>(refCams.size(), maxn);
	if (nInterp == 0) {
		return vector<size_t>();
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

	indices.resize(nInterp);
	return indices;
}

vector<float> DefaultWeightStrgFunc(const vector<Camera> &refCams,
	const Camera &virCam, const glm::vec3 &p, const vector<size_t> &indices)
{
	const size_t nInterp = indices.size();

	if (nInterp == 0) {
		return vector<float>(); 
	}

	vector<float> weights(nInterp);
	const glm::vec3 &virPos = virCam.GetPosition();
	const glm::vec3 &p2vir = glm::normalize(virPos - p);

	for (size_t i = 0; i < nInterp; ++i) {
		// calculate cosine distances
		const glm::vec3 &refPos = refCams[indices[i]].GetPosition();
		const float dis = glm::dot(p2vir, glm::normalize(refPos - p));

		// assign weights according to cosine distances
		const float weight = 1.f / (1.f + 1e-5f - dis);
		weights[i] = weight;
	}

	// normalize weights
	const float sumW = std::accumulate(weights.cbegin(), weights.cend(), 0.f);
	std::for_each(weights.begin(), weights.end(),
		[sumW](float &w) { w = w / sumW; });

	return weights;
}