#ifndef INTRINSIC_H
#define INTRINSIC_H

#include <cstdint>
#include <cmath>

class Intrinsic
{
public:
	Intrinsic()
		: mInitialized(false),
		mCx(0.0), mCy(0.0),
		mFx(0.0), mFy(0.0),
		mImgW(0), mImgH(0)
	{}

	Intrinsic(double cx, double cy, double fx, double fy)
		: mInitialized(true), mCx(cx), mCy(cy), mFx(fx), mFy(fy), 
		mImgW(static_cast<uint32_t>(cx * 2)), 
		mImgH(static_cast<uint32_t>(cy * 2))
	{}

	Intrinsic(double cx, double cy, double fx, double fy, 
		uint32_t imgw, uint32_t imgh)
		: mInitialized(true), mCx(cx), mCy(cy), mFx(fx), mFy(fy), 
		mImgW(imgw), mImgH(imgh)
	{}

	inline bool IsInitialized() const { return mInitialized; }
	inline double GetCx() const { return mCx; }
	inline double GetCy() const { return mCy; }
	inline double GetFx() const { return mFx; }
	inline double GetFy() const { return mFy; }
	inline double GetFOVW() const { return 2*std::atan2(mCx, mFx); }
	inline double GetFOVH() const { return 2*std::atan2(mCy, mFy); }
	inline uint32_t GetWidth() const { return mImgW; }
	inline uint32_t GetHeight() const { return mImgH; }

protected:
	bool mInitialized;

	/**
	 * Center of projection
	 */
	double mCx, mCy;

	/**
	 * Focal length
	 */
	double mFx, mFy;

	/**
	 * Image width and height
	 *
	 * These two properties are put here for convenience's sake
	 */
	uint32_t mImgW, mImgH;
};

#endif