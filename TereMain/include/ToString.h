#ifndef TO_STRING_H
#define TO_STRING_H

#include "Platform.h"

#if PLATFORM_ANDROID
#define TO_STRING to_stringAndroid
#include <string>
#include <sstream>
template <typename T>
inline std::string to_stringAndroid(T value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}
#else 
#include <string>
#define TO_STRING std::to_string
#endif

#endif /* TO_STRING_H */
