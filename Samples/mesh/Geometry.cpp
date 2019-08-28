#include <fstream>
#include <iostream>

#include "Geometry.hpp"
#include "tiny_obj_loader.h"
#include "tinyply.h"

using namespace std;

Geometry Geometry::FromFile(const std::string &filename)
{
	std::string ext = filename.substr(filename.length() - 3, 3);

	if (ext == "obj" || ext == "OBJ") {
		return Geometry::FromObj(filename);
	}
	else if (ext == "ply" || ext == "PLY") {
		return Geometry::FromPly(filename);
	}
	else {
		cerr << "[ERROR] Geometry: Unknown format: " << filename << endl;
	}
	return Geometry();
}

Geometry Geometry::FromObj(const std::string &filename)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error = "", warn = "";
	Geometry geometry;
	bool result = false;

	result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filename.c_str());
	
	if (!warn.empty()) {
		cout << "[WARN] Geometry: " << warn << endl;
	}

	if (!result) {
		throw std::runtime_error("[ERROR] Geometry: " + error);
	}

	bool reconstruct = false;
	/* Check whether vertices should be duplicated
	 * i.e., each face has different vertex/normal/uv indices
	 */
	for (int meshid = 0; meshid != shapes.size(); ++meshid) {
		const tinyobj::shape_t &shape = shapes[meshid];
		const tinyobj::mesh_t &mesh = shape.mesh;

		for (int i = 0; i != mesh.indices.size(); ++i) {
			if ((mesh.indices[i].normal_index != -1 && 
				mesh.indices[i].vertex_index != mesh.indices[i].normal_index) ||
				(mesh.indices[i].texcoord_index != -1 &&
				mesh.indices[i].vertex_index != mesh.indices[i].texcoord_index)) 
			{
				reconstruct = true;
				break;
			}
		}

		if (reconstruct) {
			break;
		}
	}

	/* Construct geometry */
	if (!reconstruct) {
		// copy indices
		for (int meshid = 0; meshid != shapes.size(); ++meshid) {
			const tinyobj::mesh_t &mesh = shapes[meshid].mesh;

			for (int i = 0; i != mesh.indices.size(); ++i) {
				geometry.indices.push_back(mesh.indices[i].vertex_index);
			}
		}

		// copy vertices, normals and texcoord 
		if (!attrib.vertices.empty()) {
			geometry.vertices = std::move(attrib.vertices);
		}
		if (!attrib.normals.empty()) {
			geometry.normals = std::move(attrib.normals);
		}
		if (!attrib.texcoords.empty()) {
			geometry.texCoords = std::move(attrib.texcoords);
		}

		// Recommend glDrawElement.
		geometry.drawOption = DrawOption::Element;
	}
	else {
		// reconstruct vertices, normals and texcoords
		geometry.vertices.reserve(attrib.vertices.size()*6);
		geometry.normals.reserve(attrib.normals.size()*6);
		geometry.texCoords.reserve(attrib.texcoords.size()*6);

		for (int meshid = 0; meshid != shapes.size(); ++meshid) {
			const tinyobj::mesh_t &mesh = shapes[meshid].mesh;

			for (int i = 0; i != mesh.indices.size(); ++i) {
				int vertexIndex = mesh.indices[i].vertex_index;
				int normalIndex = mesh.indices[i].normal_index;
				int texcoordIndex = mesh.indices[i].texcoord_index;

				if (vertexIndex != -1) {
					geometry.vertices.push_back(attrib.vertices[3 * (vertexIndex)]);
					geometry.vertices.push_back(attrib.vertices[3 * (vertexIndex)+1]);
					geometry.vertices.push_back(attrib.vertices[3 * (vertexIndex)+2]);
				}
				if (normalIndex != -1) {
					geometry.normals.push_back(attrib.normals[3 * (normalIndex)]);
					geometry.normals.push_back(attrib.normals[3 * (normalIndex)+1]);
					geometry.normals.push_back(attrib.normals[3 * (normalIndex)+2]);
				}
				if (texcoordIndex != -1) {
					geometry.texCoords.push_back(attrib.texcoords[2 * (texcoordIndex)]);
					geometry.texCoords.push_back(attrib.texcoords[2 * (texcoordIndex)+1]);
				}
			}
		}

		// Recommend glDrawArray; Hence no indices are needed.
		geometry.drawOption = DrawOption::Array;
	}

	return geometry;
}

Geometry Geometry::FromPly(const std::string &filename)
{
	ifstream ss(filename, ios::binary);
	Geometry geometry;
	tinyply::PlyFile file;

	if (ss.fail()) throw runtime_error("[ERROR] Geometry: failed to open " + filename);

	file.parse_header(ss);

	shared_ptr<tinyply::PlyData> vertices, colors, normals, faces, texcoords;

	try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
	catch (const std::exception & e) { throw e; }

	try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
	catch (const std::exception & e) { cout << e.what() << endl; }

	try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue" }); }
	catch (const std::exception & e) { cout << e.what() << endl; }

	try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
	catch (const std::exception & e) { throw e; }

	try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
	catch (const std::exception & e) { cout << e.what() << endl; }

	file.read(ss);

	if (vertices) {
		const size_t numVerticesBytes = vertices->buffer.size_bytes();
		geometry.vertices = vector < float >(vertices->count * 3);
		std::memcpy(geometry.vertices.data(), vertices->buffer.get(), numVerticesBytes);
	}

	if (normals) {
		const size_t numNormalsBytes = normals->buffer.size_bytes();
		geometry.normals = vector<float>(vertices->count * 3);
		std::memcpy(geometry.normals.data(), vertices->buffer.get(), numNormalsBytes);
	}

	if (texcoords) {
		const size_t numTexcoordsBytes = texcoords->buffer.size_bytes();
		geometry.texCoords = vector<float>(vertices->count * 2);
		std::memcpy(geometry.texCoords.data(), texcoords->buffer.get(), numTexcoordsBytes);
	}

	if (colors) {
		const size_t numColorsBytes = colors->buffer.size_bytes();
		geometry.colors = vector < uint8_t >(colors->count * 3);
		std::memcpy(geometry.colors.data(), colors->buffer.get(), numColorsBytes);
	}

	if (faces) {
		const size_t numFacesBytes = faces->buffer.size_bytes();
		geometry.indices = vector < int32_t >(faces->count * 3);
		std::memcpy(geometry.indices.data(), faces->buffer.get(), numFacesBytes);
	}

	geometry.drawOption = DrawOption::Element;

	return geometry;
}