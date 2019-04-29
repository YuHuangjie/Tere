#ifndef PLATFORM_H
#define PLATFORM_H

#if _MSC_VER
#	define PLATFORM_WIN
#elif defined(__APPLE__)
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
#		define PLATFORM_IPHONE
#		define PLATFORM_IOS
#	else
#		define PLATFORM_OSX
#	endif
#elif defined __ANDROID__
#	define PLATFORM_ANDROID
#else
#	error "Unrecognized platform"
#endif

#endif /* PLATFORM_H */
