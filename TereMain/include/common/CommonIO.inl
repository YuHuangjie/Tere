#include "common/CommonIO.hpp"
#include <sstream>
#include <cstdint>

using namespace std;

inline CommonIO::CommonIO()
{	}

inline int CommonIO::ReadList(const std::string &filename, std::vector<std::string>& container,
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

inline int CommonIO::WriteList(const std::string &filename,
	const vector<string> &container)
{
	ofstream outFile(filename);
	if (!outFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " + filename);
	}

	int index = 0;

	for (vector<string>::const_iterator it = container.cbegin();
		it != container.cend(); ++it) {
		outFile << index << " " << *it << endl;
		index++;
	}

	outFile.close();
	return index;
}

inline int CommonIO::ReadIntrinsic(const std::string &filename,
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

inline int CommonIO::WriteIntrinsic(const std::string &filename,
	const vector<Intrinsic> &container)
{
	ofstream outFile(filename);
	if (!outFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " + filename);
	}

	int index = 0;

	// comment
	outFile << "## index fx fy cx cy width height" << endl;

	for (vector<Intrinsic>::const_iterator it = container.cbegin();
		it != container.cend(); ++it) {
		outFile << index << " " << it->GetFx() << " " << it->GetFy() << " "
			<< it->GetCx() << " " << it->GetCy() << " " << it->GetWidth() << " "
			<< it->GetHeight() << endl;
		index++;
	}

	outFile.close();
	return index;
}

inline int CommonIO::ReadExtrinsic(const std::string &filename,
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

inline int CommonIO::WriteExtrinsic(const std::string &filename,
	const vector<Extrinsic> &container)
{
	ofstream outFile(filename);
	if (!outFile.is_open()) {
		throw runtime_error("!![Error] CommonIO: Can't open " + filename);
	}

	int index = 0;

	// comment
	outFile << "## index up dir position" << endl;

	for (vector<Extrinsic>::const_iterator it = container.cbegin();
		it != container.cend(); ++it) {
		outFile << index << " " << it->GetUp().x << " " << it->GetUp().y
			<< " " << it->GetUp().z << " " << it->GetDir().x << " "
			<< it->GetDir().y << " " << it->GetDir().z << " "
			<< it->GetPos().x << " " << it->GetPos().y << " "
			<< it->GetPos().z << endl;
		index++;
	}

	outFile.close();
	return index;
}