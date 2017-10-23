#pragma once

/* OpenGL(ES) */
#ifdef _WIN32
	#define GL
#elif __ANDROID__
	#define GL_ES3
#elif __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_OS_IPHONE
		#define GL_ES3
		#define IOS
	#else
		#error "Unknown platform"
	#endif
#else
	#error "Unknown platform"
#endif

/* LOG function */
#if defined _WIN32 || defined __APPLE__
#include <cstdio>
#define LOGI(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#elif defined __ANDROID__
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "AndroidProject1.NativeActivity", __VA_ARGS__))
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