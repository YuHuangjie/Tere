#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <string>
#include <vector>

enum DrawOption
{
	Array = 0,
	Element = 1
};

struct Geometry
{
	DrawOption drawOption;

	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<uint8_t> colors;
	std::vector<int32_t> indices;

	static Geometry FromFile(const std::string &filename);
	static Geometry FromPly(const std::string &filename);
	static Geometry FromObj(const std::string &filename);

	inline bool HasFace() const { return !indices.empty(); }
	inline bool HasColor() const { return !colors.empty(); }
	inline bool HasTexCoord() const { return !texCoords.empty(); }
	inline bool HasNormal() const { return !normals.empty(); }
	inline bool HasVertex() const { return !vertices.empty(); }
};

#endif /* GEOMETRY_H */
