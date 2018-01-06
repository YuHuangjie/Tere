#include "ObjIO.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using std::ifstream;
using std::ofstream;
using std::runtime_error;
using std::istringstream;
using std::endl;

Mesh ObjIO::ReadMesh(const string &filename)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) {
		throw runtime_error("cannot open " + filename);
	}

	Mesh mesh;
	string line = "";
	string prefix = "";
	istringstream iss;
	float x, y, z;

	while (!inFile.eof()) {
		std::getline(inFile, line);
		iss.str(line);
		iss >> prefix;

		if (prefix.find_first_of('#') == 0) {
			iss.clear();
			continue;
		}
		else if (prefix == "vn") {
			iss >> x >> y >> z;
			iss.clear();
			mesh.vn.push_back(glm::vec3(x, y, z));
		}
		else if (prefix == "v") {
			iss >> x >> y >> z;
			iss.clear();
			mesh.v.push_back(glm::vec3(x, y, z));
		}
		else if (prefix == "vt") {
			iss >> x >> y;
			iss.clear();
			mesh.vt.push_back(glm::vec2(x, y));
		}
		else if (prefix == "f") {
			mesh.faces.push_back(line);
		}
	}

	inFile.close();
	return mesh;
}

bool ObjIO::WriteMesh(const string &filename, const Mesh &mesh)
{
	ofstream outFile(filename);
	if (!outFile.is_open()) {
		throw runtime_error("cannot open " + filename);
	}

	// comment
	outFile << "####" << endl
		<< "#" << endl
		<< "# OBJ File Generated by YuHuangjie" << endl
		<< "#" << endl
		<< "####" << endl
		<< "#" << endl
		<< "# Vertices: " << mesh.v.size() << endl
		<< "# Normals: " << mesh.vn.size() << endl
		<< "# Texcoords: " << mesh.vt.size() << endl
		<< "# Faces: " << mesh.faces.size() << endl
		<< "#" << endl
		<< "####" << endl << endl;

	// output vertex
	for (vector<glm::vec3>::const_iterator it = mesh.v.cbegin();
		it != mesh.v.cend(); ++it) { 
		outFile << "v " << it->x << " " << it->y << " " << it->z << endl;
	}
	outFile << endl;

	// output Normals
	for (vector<glm::vec3>::const_iterator it = mesh.vn.cbegin();
		it != mesh.vn.cend(); ++it) {
		outFile << "vn " << it->x << " " << it->y << " " << it->z << endl;
	}
	if (mesh.vn.size() > 0) {
		outFile << endl;
	}

	// output Texcoords
	for (vector<glm::vec2>::const_iterator it = mesh.vt.cbegin();
		it != mesh.vt.cend(); ++it) {
		outFile << "vt " << it->x << " " << it->y << endl;
	}
	if (mesh.vt.size() > 0) {
		outFile << endl;
	}

	// output faces
	for (vector<string>::const_iterator it = mesh.faces.cbegin();
		it != mesh.faces.cend(); ++it) {
		outFile << *it << endl;
	}
	outFile << endl;

	outFile.close();
	return true;
}