#ifndef OBJIO_H
#define OBJIO_H

#include <string>
#include <vector>
#include "glm/glm.hpp"

using std::vector;
using std::string;


typedef struct Mesh
{
	vector<glm::vec3> v;
	vector<glm::vec3> vn;
	vector<glm::vec2> vt;
	vector<string> faces;
} Mesh;

class ObjIO
{
public:
	static Mesh ReadMesh(const string &filename);
	static bool WriteMesh(const string &filename, const Mesh&);
};

#endif
