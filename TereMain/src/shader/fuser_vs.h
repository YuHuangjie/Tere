#ifndef TEXTURE_VS_H
#define TEXTURE_VS_H

#include "common/Common.hpp"

const char *fuser_vs_code =
"//fusv\n"
#if PLATFORM_WIN || PLATFORM_OSX
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
