#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <sstream>
using namespace std;

string Usage()
{
	return 
	"This small tool provices the function to rearrange mesh's vertices order "
	"according to predefined order. \nUsually, the mesh needed rearrangement "
	"is named camera_mesh.obj, and predefined vertices order is stored "
	"in cameras.xyz.\r\n"
	"Usage:\r\n"
	"rearrange_mesh camera_mesh.obj cameras.xyz\r\n";
}

void ReadXYZ(ifstream &file, vector<glm::vec3> &points)
{
	float f1, f2, f3;
	float invalid = 1e10f;

	while (!file.eof()) {
		f1 = invalid;	f2 = invalid;	f3 = invalid;
		file >> f1 >> f2 >> f3;

		if (f1 == invalid || f2 == invalid || f3 == invalid) {
			break;
		}

		points.push_back(glm::vec3(f1, f2, f3));
	}
}

void ReadMesh(ifstream &file, vector<glm::vec3> &points, vector<glm::ivec4> &faces)
{
	string line;
	istringstream ss_line;
	float p1, p2, p3;
	int id1 = -1;
	int id2 = -1;
	int id3 = -1;
	int id4 = 0;

	while (!file.eof()) {
		getline(file, line);

		if (line.empty() || (line[0] != 'v' && line[0] != 'f')) continue;
		else if (line[0] == 'v') {
			ss_line.str(line.substr(2));
			ss_line >> p1 >> p2 >> p3;
			points.push_back(glm::vec3(p1, p2, p3));
			ss_line.clear();
		}
		else if (line[0] == 'f') {
			id1 = -1;	id2 = -1;	id3 = -1;	id4 = 0;
			ss_line.str(line.substr(2));
			ss_line >> id1 >> id2 >> id3 >> id4;
			// .obj is 1 based
			faces.push_back(glm::ivec4(id1 - 1, id2 - 1, id3 - 1, id4 - 1));
			ss_line.clear();
		}
	}
}

int VertexIndex(const vector<glm::vec3> &points, const glm::vec3 p)
{
	const float EPS = 1e-3f;

	for (vector<glm::vec3>::const_iterator cit = points.begin();
		cit != points.cend(); ++cit) {

		glm::vec3 old_p = *cit;
		if (glm::length(old_p - p) < EPS) {
			return static_cast<int>(cit - points.cbegin());
		}
	}
}

int main(int argc, char *argv[])
{
	/* 
	 * We need two files, one is camera_mesh.obj who needs rearrangement,
	 * the other one is cameras.xyz which defines correct order of vertices
	 */
	string mesh_file_name;
	string order_file_name;

	if (argc != 3) {
		cerr << Usage() << endl;
		exit(-1);
	}
	else {
		mesh_file_name = string(argv[1]);
		order_file_name = string(argv[2]);
	}

	// check file validity
	ifstream mesh_file(mesh_file_name);
	ifstream order_file(order_file_name);
	if (!mesh_file.is_open()) {
		cerr << "Unable to open " << mesh_file_name << endl;
		exit(-1);
	}
	if (!order_file.is_open()) {
		cerr << "Unable to open " << order_file_name << endl;
		exit(-1);
	}

	// read in predefined vertices order
	vector<glm::vec3> predefined_order;
	ReadXYZ(order_file, predefined_order);

	// read in current vertice order and faces
	vector<glm::vec3> current_order;
	vector<glm::ivec4> current_faces;
	ReadMesh(mesh_file, current_order, current_faces);


	/*				rearragement			*/
	ofstream new_mesh_file("rearranged.obj");
	new_mesh_file << "####" << endl
		<< "#" << endl
		<< "# OBJ File Generated by YuHuangjie" << endl
		<< "#" << endl
		<< "####" << endl
		<< "#" << endl
		<< "# Vertices: " << predefined_order.size() << endl
		<< "# Faces: " << current_faces.size() << endl
		<< "#" << endl
		<< "####" << endl;

	// output new vertices 
	for (vector<glm::vec3>::iterator it = predefined_order.begin();
		it != predefined_order.end(); ++it) {
		new_mesh_file << "v " << (*it)[0] << " " << (*it)[1] << " " << (*it)[2] << endl;
	}
	new_mesh_file << "# " << predefined_order.size() << " vertices" << endl << endl;

	// output new faces
	for (vector<glm::ivec4>::iterator it = current_faces.begin();
		it != current_faces.end(); ++it) {

		bool is_quad = ((*it).w != -1);
		glm::vec3 old_v1 = current_order[(*it).x];
		glm::vec3 old_v2 = current_order[(*it).y];
		glm::vec3 old_v3 = current_order[(*it).z];
		glm::vec3 old_v4 = (is_quad ? current_order[(*it).w] : glm::vec3());

		// locate old vertices in predefined order
		new_mesh_file << "f "
			<< VertexIndex(predefined_order, old_v1) + 1 << " "
			<< VertexIndex(predefined_order, old_v2) + 1 << " "
			<< VertexIndex(predefined_order, old_v3) + 1;
		if (is_quad) {
			new_mesh_file << " " << VertexIndex(predefined_order, old_v4) + 1;
		}
		new_mesh_file << endl;
	}
	new_mesh_file << "#" << current_faces.size() << " faces" << endl << endl
		<< "# End of File" << endl;

	// release file handles
	mesh_file.close();
	order_file.close();
}