#ifndef GL_HEADER_H
#define GL_HEADER_H

#include "Platform.h"

#if defined PLATFORM_WIN
	#include <GL/glew.h>
	#define GL_WIN
#elif defined PLATFORM_OSX
	#include <GL/glew.h>
	#define GL_OSX
#elif defined PLATFORM_IOS
	#include <OpenGLES/ES3/gl.h>
	#include <OpenGLES/ES3/glext.h>
	#define GL_IOS	
#elif PLATFORM_ANDROID
	#include <GLES3/gl3.h>
	#include <GLES2/gl2ext.h>
	#define GL_ANDROID
#endif

#endif /* */
