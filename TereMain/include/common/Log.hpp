#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#ifdef USE_LOG

#include "Platform.h"

/* LOG function */
#if defined PLATFORM_WIN ||defined PLATFORM_IPHONE ||defined PLATFORM_OSX
#include <cstdio>
#define LOGI(...) fprintf(stdout, __VA_ARGS__)
#define vLOGI(format, arg_list) vprintf(format, arg_list)
#define LOGD  LOGI
#define vLOGD vLOGI
#define LOGW  LOGI
#define vLOGW vLOGI
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#define vLOGE(format, arg_list) vprintf(format, arg_list)
#elif defined PLATFORM_ANDROID
#include <android/log.h>
#define TAG "TERE"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define vLOGI(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_INFO, TAG, format, arg_list))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#define vLOGW(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_WARN, TAG, format, arg_list))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
#define vLOGD(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_DEBUG, TAG, format, arg_list))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#define vLOGE(format, arg_list) ((void)__android_log_vprint(ANDROID_LOG_ERROR, TAG, format, arg_list))
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

#endif /* COMMON_LOG_H */
