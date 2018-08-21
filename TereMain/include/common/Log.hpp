#ifndef LOG_H
#define LOG_H

#if ENABLE_LOG

#include "Common.hpp"

/* LOG function */
#if PLATFORM_WIN || PLATFORM_IPHONE || PLATFORM_OSX
#include <cstdio>
#define LOGI(...) printf(__VA_ARGS__)
#define vLOGI(format, arg_list) vprintf(format, arg_list)
#define LOGD(...) printf(__VA_ARGS__)
#define vLOGD(format, arg_list) vprintf(format, arg_list)
#define LOGW(...) printf(__VA_ARGS__)
#define vLOGW(format, arg_list) vprintf(format, arg_list)
#define LOGE(...) printf(__VA_ARGS__)
#define vLOGE(format, arg_list) vprintf(format, arg_list)
#elif defined PLATFORM_ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ObjectScanning", __VA_ARGS__))
#define vLOGI(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_INFO, "ObjectScanning", format, arg_list))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ObjectScanning", __VA_ARGS__))
#define vLOGW(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_WARN, "ObjectScanning", format, arg_list))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "ObjectScanning", __VA_ARGS__))
#define vLOGD(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_DEBUG, "ObjectScanning", format, arg_list))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ObjectScanning", __VA_ARGS__))
#define vLOGE(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_ERROR, "ObjectScanning", format, arg_list))
#endif

#else
#define LOGI(...) 0
#define vLOGI(format, arg_list) 0
#define LOGD(...) 0
#define vLOGD(format, arg_list) 0
#define LOGW(...) 0
#define vLOGW(format, arg_list) 0
#define LOGE(...) 0
#define vLOGE(format, arg_list) 0

#endif // ENABLE_LOG

#endif