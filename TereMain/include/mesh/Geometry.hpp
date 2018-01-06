#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>
#include "tiny_obj_loader.h"

enum DrawOption
{
	Array = 0,
	Element = 1
};

class Geometry
{
public:
	Geometry();

	static Geometry FromObj(const std::string &filename);

	inline DrawOption GetDrawOption(void) const { 
		return mDrawOption; 
	}
	inline const std::vector<float> &GetVertices(void) const {
		return mVertices; 
	}
	inline const std::vector<float> &GetNormals(void) const {
		return mNormals;
	}
	inline const std::vector<float> &GetTexcoords(void) const {
		return mTexcoords;
	}
	inline const std::vector<int32_t> &GetIndices(void) const {
		return mIndices;
	}

protected:
	DrawOption mDrawOption;
	std::vector<float> mVertices;
	std::vector<float> mNormals;
	std::vector<float> mTexcoords;
	std::vector<int32_t> mIndices;
};

#include "Geometry.inl"

#endif