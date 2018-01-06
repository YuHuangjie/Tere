#ifndef COMMON_H
#define COMMON_H

// Platform recognition
#if _MSC_VER
#	define PLATFORM_WIN 1
#	define GL_WIN 1
#elif defined(__APPLE__)
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
#		define PLATFORM_IPHONE 1
#		define PLATFORM_IOS 1
#		define GL_ES3_IOS 1
#		define GL_ES3_IPHONE 1
#	else
#		define PLATFORM_OSX 1
#		define GL_OSX 1
#	endif
#elif defined __ANDROID__
#	define PLATFORM_ANDROID 1
#	define GL_ES3_ANDROID 1
#else
#	error "Unrecognized platform"
#endif

// TO_STRING
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

#endif