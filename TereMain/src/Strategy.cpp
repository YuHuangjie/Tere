#include <algorithm>
#include <numeric>

#include "Strategy.h"
#include "WeightedCamera.h"
#include "camera/Extrinsic.hpp"

using std::vector;

/******************************************************************************
 *				Searching stratergy
 *****************************************************************************/

vector<size_t> DefaultSearchStrgFunc(const vector<Extrinsic> &ref,
	const Extrinsic &rcam, const size_t maxn, void *data)
{
	const size_t nInterp = std::min<size_t>(ref.size(), maxn);
	vector<float> distances;
	
	if (nInterp == 0) {
		return vector<size_t>();
	}

	// calculate cosine distances between virtual camera and every
	// reference camera
	const Extrinsic rEx(rcam);
	float *center = static_cast<float*>(data);
	const glm::vec3 p(center[0], center[1], center[2]);
	const glm::vec3 &rpos = rEx.Pos();
	const glm::vec3 &p2r = glm::normalize(rpos - p);

	for (auto c : ref) {
		const Extrinsic refEx(c);
		const glm::vec3 &refPos = refEx.Pos();
		const float dis = glm::dot(p2r, glm::normalize(refPos - p));
		distances.push_back(dis);
	}

	vector<size_t> indices(ref.size());
	std::iota(indices.begin(), indices.end(), 0);

	// find reference cameras that have least cosine distances
	std::partial_sort(indices.begin(), indices.begin() + nInterp,
		indices.end(), [&distances](const size_t &a, const size_t &b) {
		return distances[a] > distances[b];
	});

	indices.resize(nInterp);
	return indices;
}

vector<size_t> AllSearchStrgFunc(const vector<Extrinsic>& ref, 
	const Extrinsic & rcam, const size_t maxn, void *data)
{
// Return all reference cameras
	vector<size_t> indices(ref.size());
	std::iota(indices.begin(), indices.end(), 0);
	return indices;
}

/******************************************************************************
 *				Weighing stratergy
 *****************************************************************************/

vector<float> DefaultWeightStrgFunc(const vector<Extrinsic> &ref,
	const Extrinsic &rcam, const float center[3], const vector<size_t> &indices)
{
	const size_t nInterp = indices.size();
	vector<float> weights(nInterp);
	const Extrinsic rEx(rcam);
	const glm::vec3 &rpos = rEx.Pos();
	const glm::vec3 p(center[0], center[1], center[2]);
	const glm::vec3 &p2r = glm::normalize(rpos - p);

	if (nInterp == 0) {
		return vector<float>(); 
	}

	for (size_t i = 0; i < nInterp; ++i) {
		// calculate cosine distances between rendering camera and 
		// i-th selected reference camera
		const Extrinsic refEx(ref[indices[i]]);
		const glm::vec3 &refPos = refEx.Pos();
		const float dis = glm::dot(p2r, glm::normalize(refPos - p));

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
