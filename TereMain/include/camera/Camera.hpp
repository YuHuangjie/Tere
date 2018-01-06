#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Intrinsic.hpp"
#include "Extrinsic.hpp"

class Camera
{
public:
	Camera();
	Camera(const Extrinsic&, const Intrinsic&);

	void SetExtrinsic(const Extrinsic&);
	void SetIntrinsic(const Intrinsic&);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float near=0.1f, float far=1000.f) const;

	inline Extrinsic GetExtrinsic(void) const {
		return mExtrinsic; 
	}
	inline Intrinsic GetIntrinsic(void) const {
		return mIntrinsic;
	}
	inline glm::vec3 GetRight(void)  const { 
		return mExtrinsic.GetRight(); 
	}
	inline glm::vec3 GetPosition(void)  const { 
		return mExtrinsic.GetPos();	
	}
	inline glm::vec3 GetDir(void)  const { 
		return mExtrinsic.GetDir(); 
	}
	inline glm::vec3 GetUp(void)  const { 
		return mExtrinsic.GetUp(); 
	}
	inline double GetFOVH(void)  const {	
		return mIntrinsic.GetFOVH(); 
	}
	inline double GetFOVW(void)  const {
		return mIntrinsic.GetFOVW();
	}
	inline double GetCx(void)  const {
		return mIntrinsic.GetCx();
	}
	inline double GetCy(void)  const {
		return mIntrinsic.GetCy(); 
	}
	inline double GetFx(void)  const {
		return mIntrinsic.GetFx();
	}
	inline double GetFy(void)  const {
		return mIntrinsic.GetFy(); 
	}
	inline double GetWidth(void)  const {
		return mIntrinsic.GetWidth(); 
	}
	inline double GetHeight(void)  const { 
		return mIntrinsic.GetHeight(); 
	}

protected:
	Extrinsic mExtrinsic;
	Intrinsic mIntrinsic;
};

#include "Camera.inl"

#endif