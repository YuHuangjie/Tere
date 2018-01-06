#ifndef LFLoader_H
#define LFLoader_H

#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <map>
#include <memory>

#include "LightFieldAttrib.h"

using std::string;
using std::vector;
using std::map;
using std::unique_ptr;
using std::runtime_error;
using std::istringstream;
using std::pair;
using std::ifstream;
using std::getline;

class LFLoader
{
public:
	explicit LFLoader(const string &profile);
	LFLoader(const LFLoader &) = delete;
	LFLoader& operator=(const LFLoader&) = delete;

	void LoadProfile(const string &profile_prefix, 
		const map<string, string> &profile);

	inline LightFieldAttrib& GetLightFieldAttrib(void) { 
		return attrib;
	};

private:
	// light field configuration
	LightFieldAttrib attrib;
};


#endif
