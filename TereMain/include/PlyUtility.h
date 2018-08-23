#ifndef PLY_UTILITY_H
#define PLY_UTILITY_H

#include <string>
#include <vector>
#include <cstdint>

bool ReadPly(const std::string &filepath,
	std::vector<float> &vertices,
	std::vector<float> &normals,
	std::vector<float> &texcoords,
	std::vector<uint8_t> &colors,
	std::vector<int32_t> &faces);


#endif