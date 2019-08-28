#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string>
#include <vector>
#include "camera/Extrinsic.hpp"

class UserInterface
{
public:
	virtual ~UserInterface() {}

	virtual std::string Name() const = 0;
	virtual void SetResolution(const float width, const float height) = 0;
    virtual void Touch(const float x, const float y) = 0;
	virtual Extrinsic Leave(const float x, const float y, const Extrinsic &cur) = 0;
	virtual Extrinsic Move(const float x, const float y, const Extrinsic &cur) = 0;
	virtual std::vector<size_t> HintInterp() = 0;
};

#endif /* USER_INTERFACE_H */
