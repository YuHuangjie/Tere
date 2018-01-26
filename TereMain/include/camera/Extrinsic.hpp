#ifndef EXTRINSIC_H
#define EXTRINSIC_H

#include <glm/glm.hpp>
#include <stdexcept>

class Extrinsic
{
public:
	Extrinsic()
		: mInitialized(false)
	{}

	Extrinsic(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
		: mInitialized(true)
	{
		const glm::vec3 dir = pos - target;
		// Perpendiclar condition
		if (glm::dot(dir, up) > 1e-5) {
			throw std::invalid_argument("dir not perpendicular to up");
			return;
		}

		mUp = glm::normalize(up);
		mDir = glm::normalize(dir);
		mRight = glm::cross(up, dir);
		mPos = pos;
	}

	inline bool IsInitialized() const { return mInitialized; }
	inline glm::vec3 GetRight() const { return mRight; }
	inline glm::vec3 GetUp() const { return mUp; }
	inline glm::vec3 GetDir() const { return mDir; }
	inline glm::vec3 GetPos() const { return mPos; }

protected:	
	bool mInitialized;

	/**
	 * Right axis
	 */
	glm::vec3 mRight;

	/**
	 * UP axis
	 */
	glm::vec3 mUp;

	/**
	 * Direction (The reverse of the direction camera is pointing at)
	 */
	glm::vec3 mDir;

	/**
	 * Camera position
	 */
	glm::vec3 mPos;
};

#endif