#include "common/CommonIO.hpp"
#include "ObjIO.h"
#include "LFLoader.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <limits>

using std::cout;
using std::endl;
using std::cin;

void Usage(void)
{
	cout << "This is an auxiliary program with 2.8D for ARKit. This program\n" 
		<< "reads a standard 2.8D profile, then scales and relocates object,\n"
		<< "camera_mesh and extrinsicx. The output object will stand on z=0\n"
		<< "plane."
		<< "    You have to provide a valid 2.8D profile text and a desired \n"
		<< "real-world height. The value determines the biggest z value of \n"
		<< "the final object.\n\n"
		<< "Usage: Scale <profile>" << endl;
}

int main(int argc, char *argv[])
{
	string profileName;		// required
	float height = 1.f;      // required
	const string outExtrinsics = "extrinsics.txt";
	const string outObject = "object.obj";
	const string outCamMesh = "camera_mesh.obj";

	// Parse arguments
	if (argc != 2) {
		Usage();
		return -1;
	}

	profileName = argv[1];

	// Ask for scale
	do {
		string sHeight;
		cout << "Desired height [default 1.0]: ";
		cin >> sHeight;
		try {
			height = std::stof(sHeight);
			break;
		}
		catch (std::invalid_argument &e) {
			cout << "Invalid scale" << endl;
			continue;
		}
	} while (true);

	try {
		// Read profile
		LFLoader loader(profileName);
		LightFieldAttrib attrib = loader.GetLightFieldAttrib();

		Mesh object = ObjIO::ReadMesh(attrib.obj_file);
		Mesh cameraMesh = ObjIO::ReadMesh(attrib.camera_mesh_name);
		vector<Camera> cameras = attrib.ref_cameras;
		vector<Extrinsic> extrinsics;

		for (vector<Camera>::const_iterator it = cameras.cbegin();
			it != cameras.cend(); ++it) {
			extrinsics.push_back(it->GetExtrinsic());
		}

		// Compute transform matrix
		float minz = std::numeric_limits<float>::max();
		float maxz = std::numeric_limits<float>::min();

		for (vector<glm::vec3>::const_iterator it = object.v.cbegin();
			it != object.v.cend(); ++it) {
			if (it->z < minz) {
				minz = it->z;
			}
			if (it->z > maxz) {
				maxz = it->z;
			}
		}

		float deltaZ = -minz;
		float scale = height / (maxz - minz);
		glm::mat4 transform(1.0f);

		transform = glm::scale(transform, glm::vec3(scale, scale, scale));
		transform = glm::translate(transform, glm::vec3(0.f, 0.f, deltaZ));

		// Compute new object
		for (vector<glm::vec3>::iterator it = object.v.begin();
			it != object.v.end(); ++it) {
			*it = glm::vec3(transform * glm::vec4(*it, 1.0f));
		}

		// Compute new camera mesh
		for (vector<glm::vec3>::iterator it = cameraMesh.v.begin();
			it != cameraMesh.v.end(); ++it) {
			*it = glm::vec3(transform * glm::vec4(*it, 1.0f));
		}

		// Compute new extrinsics
		for (vector<Extrinsic>::iterator it = extrinsics.begin();
			it != extrinsics.end(); ++it) {
			glm::vec3 newPos(transform * glm::vec4(it->GetPos(), 1.0f));
			*it = Extrinsic(newPos, newPos - it->GetDir(), it->GetUp());
		}

		// Write object, camera mesh and extrinsics
		CommonIO::WriteExtrinsic(outExtrinsics, extrinsics);
		ObjIO::WriteMesh(outObject, object);
		ObjIO::WriteMesh(outCamMesh, cameraMesh);
	}
	catch (std::exception &e) {
		std::cerr << "Catch exception: " << e.what() << endl;
	}
}