#ifndef LOG_H
#define LOG_H

#if ENABLE_LOG

#include "Preprocess.h"

/* LOG function */
#if PLATFORM_WIN || PLATFORM_IPHONE
#include <cstdio>
#define LOGI(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#elif defined PLATFORM_ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "decodetag", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "decodetag", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "decodetag", __VA_ARGS__))
#endif

#else
#define LOGI(...)
#define LOGD(...)
#define LOGW(...)

#endif // ENABLE_LOG

#endif