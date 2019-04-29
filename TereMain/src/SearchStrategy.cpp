#include <algorithm>
#include <numeric>
#include "Strategy.h"

/******************************************************************************
 *				Default searching stratergy
 *****************************************************************************/


DefaultSearchStrategy::DefaultSearchStrategy(const glm::vec3 &center)
	: _center(center)
{}

vector<size_t> DefaultSearchStrategy::Search(const vector<Extrinsic> &ref,
	const Extrinsic &rcam, const size_t maxn)
{
	const size_t nInterp = std::min<size_t>(ref.size(), maxn);
	vector<float> distances;

	if (nInterp == 0) {
		return vector<size_t>();
	}

	// calculate cosine distances between virtual camera and every
	// reference camera
	const Extrinsic rEx(rcam);
	const glm::vec3 &rpos = rEx.Pos();
	const glm::vec3 &p2r = glm::normalize(rpos - _center);

	for (auto c : ref) {
		const Extrinsic refEx(c);
		const glm::vec3 &refPos = refEx.Pos();
		const float dis = glm::dot(p2r, glm::normalize(refPos - _center));
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

/******************************************************************************
 *				All searching stratergy
 *****************************************************************************/

AllSearchStrategy::AllSearchStrategy() {}

vector<size_t> AllSearchStrategy::Search(const vector<Extrinsic> &ref,
	const Extrinsic &rcam, const size_t maxn)
{
// Return all reference cameras
	const size_t nInterp = std::min<size_t>(ref.size(), maxn);
	vector<size_t> indices(nInterp);
	std::iota(indices.begin(), indices.end(), 0);
	return indices;
}