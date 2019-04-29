#ifndef POSTER_VS_H
#define POSTER_VS_H

#include "Platform.h"

const char *poster_vs_code =
"//posv\n"
#if defined PLATFORM_WIN ||defined PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"layout(location = 0) in vec2 aPos;		\n"
"layout(location = 1) in vec2 aTexCoord;\n"

"out vec2 vTexCoord;						\n"

"void main()							\n"
"{										\n"
"	gl_Position = vec4(aPos, 0.0, 1.0);		\n"
"	vTexCoord = aTexCoord;	\n"
"}	\n";

#endif
