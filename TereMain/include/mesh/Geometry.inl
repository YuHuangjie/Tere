#include "Geometry.hpp"

inline Geometry::Geometry() 
{}

inline Geometry Geometry::FromObj(const std::string &filename)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error = "";
	Geometry geometry;

	bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, filename.c_str());
	if (!result) {
		throw std::runtime_error("[Mesh] " + error);
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
				geometry.mIndices.push_back(mesh.indices[i].vertex_index);
			}
		}

		// copy vertices, normals and texcoord 
		if (!attrib.vertices.empty()) {
			geometry.mVertices = std::move(attrib.vertices);
		}
		if (!attrib.normals.empty()) {
			geometry.mNormals = std::move(attrib.normals);
		}
		if (!attrib.texcoords.empty()) {
			geometry.mTexcoords = std::move(attrib.texcoords);
		}

		// Recommend glDrawElement.
		geometry.mDrawOption = DrawOption::Element;
	}
	else {
		// reconstruct vertices, normals and texcoords
		geometry.mVertices.reserve(attrib.vertices.size()*6);
		geometry.mNormals.reserve(attrib.normals.size()*6);
		geometry.mTexcoords.reserve(attrib.texcoords.size()*6);

		for (int meshid = 0; meshid != shapes.size(); ++meshid) {
			const tinyobj::mesh_t &mesh = shapes[meshid].mesh;

			for (int i = 0; i != mesh.indices.size(); ++i) {
				int vertexIndex = mesh.indices[i].vertex_index;
				int normalIndex = mesh.indices[i].normal_index;
				int texcoordIndex = mesh.indices[i].texcoord_index;

				if (vertexIndex != -1) {
					geometry.mVertices.push_back(attrib.vertices[3 * (vertexIndex)]);
					geometry.mVertices.push_back(attrib.vertices[3 * (vertexIndex)+1]);
					geometry.mVertices.push_back(attrib.vertices[3 * (vertexIndex)+2]);
				}
				if (normalIndex != -1) {
					geometry.mNormals.push_back(attrib.normals[3 * (normalIndex)]);
					geometry.mNormals.push_back(attrib.normals[3 * (normalIndex)+1]);
					geometry.mNormals.push_back(attrib.normals[3 * (normalIndex)+2]);
				}
				if (texcoordIndex != -1) {
					geometry.mTexcoords.push_back(attrib.texcoords[2 * (texcoordIndex)]);
					geometry.mTexcoords.push_back(attrib.texcoords[2 * (texcoordIndex)+1]);
				}
			}
		}

		// Recommend glDrawArray; Hence no indices are needed.
		geometry.mDrawOption = DrawOption::Array;
	}

	return geometry;
}