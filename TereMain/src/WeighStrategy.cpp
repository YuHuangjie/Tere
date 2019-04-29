#include <numeric>
#include <algorithm>
#include "Strategy.h"

DefaultWeighStrategy::DefaultWeighStrategy(const glm::vec3 &center)
	: WeighStrategy(),
	_center(center)
{}

vector<float> DefaultWeighStrategy::Weigh(const vector<Extrinsic> &ref,
	const Extrinsic &rcam, const vector<size_t> &indices)
{
	const size_t nInterp = indices.size();
	vector<float> weights(nInterp);
	const Extrinsic rEx(rcam);
	const glm::vec3 &rpos = rEx.Pos();
	const glm::vec3 &p2r = glm::normalize(rpos - _center);

	if (nInterp == 0) {
		return vector<float>();
	}

	for (size_t i = 0; i < nInterp; ++i) {
		// calculate cosine distances between rendering camera and 
		// i-th selected reference camera
		const Extrinsic refEx(ref[indices[i]]);
		const glm::vec3 &refPos = refEx.Pos();
		const float dis = glm::dot(p2r, glm::normalize(refPos - _center));

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
