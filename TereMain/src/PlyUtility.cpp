#include <fstream>
#include <stdexcept>
#include <cstring>

#include "PlyUtility.h"
#include "tinyply.h"
#include "common/Log.hpp"

using namespace tinyply;
using std::vector;
using std::string;
using std::shared_ptr;
using std::ifstream;
using std::runtime_error;
using std::exception;
using std::ios;


bool ReadPly(const std::string &filepath,
	std::vector<float> &_vertices,
	std::vector<float> &_normals,
	std::vector<float> &_texcoords,
	std::vector<uint8_t> &_colors,
	std::vector<int32_t> &_faces)
{
	try {
		ifstream ss(filepath, ios::binary);
		if (ss.fail()) throw runtime_error("failed to open " + filepath);

		PlyFile file;
		file.parse_header(ss);

		shared_ptr<PlyData> vertices, colors, normals, faces, texcoords;

		try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception & e) { LOGD("tinyply exception: %s\n", e.what()); }

		try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
		catch (const std::exception & e) { LOGD("tinyply exception: %s\n", e.what()); }

		try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue" }); }
		catch (const std::exception & e) { LOGD("tinyply exception: %s\n", e.what()); }
		
		try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
		catch (const std::exception & e) { LOGD("tinyply exception: %s\n", e.what()); }

		try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
		catch (const std::exception & e) { LOGD("tinyply exception: %s\n", e.what()); }

		file.read(ss);

		if (vertices) {
			const size_t numVerticesBytes = vertices->buffer.size_bytes();
			_vertices = vector < float >(vertices->count * 3);
			std::memcpy(_vertices.data(), vertices->buffer.get(), numVerticesBytes);
		}

		if (normals) {
			const size_t numNormalsBytes = normals->buffer.size_bytes();
			_normals = vector<float>(vertices->count * 3);
			std::memcpy(_normals.data(), vertices->buffer.get(), numNormalsBytes);
		}

		if (texcoords) {
			const size_t numTexcoordsBytes = texcoords->buffer.size_bytes();
			_texcoords = vector<float>(vertices->count * 2);
			std::memcpy(_texcoords.data(), texcoords->buffer.get(), numTexcoordsBytes);
		}

		if (colors) {
			const size_t numColorsBytes = colors->buffer.size_bytes();
			_colors = vector < uint8_t >(colors->count * 3);
			std::memcpy(_colors.data(), colors->buffer.get(), numColorsBytes);
		}

		if (faces) {
			const size_t numFacesBytes = faces->buffer.size_bytes();
			_faces = vector < int32_t >(faces->count * 3);
			std::memcpy(_faces.data(), faces->buffer.get(), numFacesBytes);
		}
	}
	catch (exception &e) {
		throw e;
	}
	return true;
}
