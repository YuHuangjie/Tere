#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <vector>
#include <glm/glm.hpp>

using std::vector;
using glm::vec3;
using glm::ivec4;

int RayQuadIntersect(const vector<vec3> &nodes,
	const vector<ivec4> &quads, const vec3 &origin, const vec3 &dir);

#endif