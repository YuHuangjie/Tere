#ifndef ARCBALLUI_H
#define ARCBALLUI_H

#include <vector>
#include <memory>

#include "UserInterface.h"

class ArcBall_t;
union Matrix3f_t;

class ArcballUI : public UserInterface
{
public:
	ArcballUI(const float width, const float height, const glm::vec3 &meshCenter);

	virtual std::string Name() const override;
	virtual void SetResolution(const float width, const float height) override;
	virtual void Touch(const float x, const float y) override;
	virtual Extrinsic Leave(const float x, const float y, const Extrinsic &view) override;
	virtual Extrinsic Move(const float x, const float y, const Extrinsic &view) override;
	virtual std::vector<size_t> HintInterp() override;

private:
	std::shared_ptr<ArcBall_t> _arcball;
	std::shared_ptr<Matrix3f_t> _rot;
	const glm::vec3 _center;
};

#endif /* ARCBALLUI_H */
