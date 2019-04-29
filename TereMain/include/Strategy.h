#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include "camera/Extrinsic.hpp"

using std::vector;

/******************************************************************************
 *				Searching stratergy
 *****************************************************************************/
class SearchStrategy
{
public:
	virtual ~SearchStrategy() {};

	virtual vector<size_t> Search(const vector<Extrinsic> &refs, 
		const Extrinsic &rcam, const size_t maxn) = 0;
};

class DefaultSearchStrategy : public SearchStrategy
{
public:
	DefaultSearchStrategy(const glm::vec3 &center);

	virtual vector<size_t> Search(const vector<Extrinsic> &refs,
		const Extrinsic &rcam, const size_t maxn) override;

private:
	glm::vec3 _center;
};

class AllSearchStrategy : public SearchStrategy
{
public:
	AllSearchStrategy();

	virtual vector<size_t> Search(const vector<Extrinsic> &refs,
		const Extrinsic &rcam, const size_t maxn) override;
};

/******************************************************************************
 *				Weighing stratergy
 *****************************************************************************/
class WeighStrategy
{
public:
	virtual ~WeighStrategy() {};

	virtual vector<float> Weigh(const std::vector<Extrinsic> &ref,
		const Extrinsic &rcam, const std::vector<size_t> &indices) = 0;
};

class DefaultWeighStrategy : public WeighStrategy
{
public:
	DefaultWeighStrategy(const glm::vec3 &center);

	virtual vector<float> Weigh(const std::vector<Extrinsic> &ref,
		const Extrinsic &rcam, const std::vector<size_t> &indices) override;

private:
	glm::vec3 _center;
};

#endif /* STRATEGY_H */
