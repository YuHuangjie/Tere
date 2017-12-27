#pragma once

/* OpenGL(ES) */
#ifdef _WIN32
	#define GL 1
    # define PLATFORM_WIN 1
#elif __ANDROID__
	#define GL_ES3 1
    #define PLATFORM_ANDROID 1
#elif __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_OS_IPHONE
		#define GL_ES3 1
		#define PLATFORM_IOS 1
        #define PLATFORM_IPHONE 1
	#else
		#error "Mac not supported"
	#endif
#else
	#error "Unknown platform"
#endif

/* ANDROID flaw */
#if defined __ANDROID__
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
#define TO_STRING std::to_string
#endif