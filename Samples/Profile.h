#ifndef PROFILE_H
#define PROFILE_H

#include <vector>
#include <string>
#include <array>
#include "Type.h"

struct Profile
{
	size_t nCams;

	std::vector< std::array<float, 16> > extrins;
	std::vector< std::array<float, 9> > intrins;
	std::vector<std::string> imageList;
	std::string mesh;

	RENDER_MODE mode;

	size_t rows;		// ONLY in LINEAR mode
};

#endif /* PROFILE_H */
