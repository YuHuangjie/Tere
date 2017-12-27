#include "common/CommonIO.h"
#include <sstream>
#include <cstdint>

using namespace std;

CommonIO::CommonIO()
{	}

int CommonIO::ReadList(const std::string &filename, std::vector<std::string>& container,
	bool addPrefix)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " +	filename);
	}

	string::size_type prefixPos = filename.find_last_of("/\\");
	string prefix = filename.substr(0, prefixPos) + "/";
	int maxIndex = 0;
	int nEntries = 0;

	while (!inFile.eof()) {
		int index;
		string line, value;
		istringstream iss;

		std::getline(inFile, line);
		if (line.empty()) continue;
		if (line.find_first_not_of(" \r\n\t") == std::string::npos) {
			continue;
		}
		if (line[0] == '#') continue;

		iss.str(line);
		iss >> index >> value;
		iss.clear();
		iss.str("");

		if (index > maxIndex) {
			maxIndex = index;
		}
		while (index + 1 > static_cast<int>(container.size())) {
			container.resize(container.size() * 2 + 1);
		}

		container[index] = addPrefix ? prefix + value : value;
		nEntries++;
	}

	container.resize(maxIndex + 1);

	return nEntries;
}

int CommonIO::ReadIntrinsic(const std::string &filename, 
	vector<Intrinsic> &container)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " + filename);
	}

	int nEntries = 0;
	int maxIndex = 0;

	while (!inFile.eof()) {
		int idx;
		double fx, fy, cx, cy;
		uint32_t imgw, imgh;
		string line;
		istringstream iss;

		std::getline(inFile, line);
		if (line.empty()) continue;
		if (line.find_first_not_of(" \r\n\t") == std::string::npos) {
			continue;
		}
		if (line[0] == '#') continue;

		iss.str(line);
		iss >> idx >> fx >> fy >> cx >> cy >> imgw >> imgh;
		iss.clear();
		iss.str("");

		if (idx > maxIndex) {
			maxIndex = idx;
		}
		while (idx + 1 > static_cast<int>(container.size())) {
			container.resize(container.size() * 2 + 1);
		}

		container[idx] = Intrinsic(cx, cy, fx, fy, imgw, imgh);
		nEntries++;
	}

	container.resize(maxIndex + 1);

	return nEntries;
}

int CommonIO::ReadExtrinsic(const std::string &filename, 
	vector<Extrinsic> &container)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " + filename);
	}

	int nEntries = 0;
	int maxIndex = 0;

	while (!inFile.eof()) {
		int idx;
		glm::vec3 up, dir, pos;
		string line;
		istringstream iss;

		std::getline(inFile, line);
		if (line.empty()) continue;
		if (line.find_first_not_of(" \r\n\t") == std::string::npos) {
			continue;
		}
		if (line[0] == '#') continue;

		iss.str(line);
		iss >> idx >> up.x >> up.y >> up.z >> dir.x >> dir.y >> dir.z 
			>> pos.x >> pos.y >> pos.z;
		iss.clear();
		iss.str("");

		if (idx > maxIndex) {
			maxIndex = idx;
		}
		while (idx + 1 > static_cast<int>(container.size())) {
			container.resize(container.size() * 2 + 1);
		}

		container[idx] = Extrinsic(pos, pos - dir, up);
		nEntries++;
	}

	container.resize(maxIndex + 1);

	return nEntries;
}