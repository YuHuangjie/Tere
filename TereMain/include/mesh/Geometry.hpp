#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>

enum DrawOption
{
	Array = 0,
	Element = 1
};

class Geometry
{
public:
	Geometry();

	static Geometry FromPly(const std::string &filename);
	static Geometry FromObj(const std::string &filename);

	inline DrawOption GetDrawOption(void) const { 
		return mDrawOption; 
	}
	inline const std::vector<float> &GetVertices(void) const {
		return mVertices; 
	}
	inline const bool HasVertex(void) const {
		return !mVertices.empty();
	}

	inline const std::vector<float> &GetNormals(void) const {
		return mNormals;
	}
	inline const bool HasNormal(void) const {
		return !mNormals.empty();
	}

	inline const std::vector<float> &GetTexcoords(void) const {
		return mTexcoords;
	}
	inline const bool HasTexcoords(void) const {
		return !mTexcoords.empty();
	}

	inline const std::vector<int32_t> &GetIndices(void) const {
		return mIndices;
	}
	inline const bool HasIndices(void) const {
		return !mIndices.empty();
	}

	inline const std::vector<uint8_t> &GetColors(void) const {
		return mColors;
	}
	inline const bool HasColors(void) const {
		return !mColors.empty();
	}

protected:
	DrawOption mDrawOption;
	std::vector<float> mVertices;
	std::vector<float> mNormals;
	std::vector<float> mTexcoords;
	std::vector<uint8_t> mColors;
	std::vector<int32_t> mIndices;
};

#include "Geometry.inl"

#endif