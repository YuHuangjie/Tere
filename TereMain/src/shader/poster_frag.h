#ifndef POSTER_FRAG_H
#define POSTER_FRAG_H

#include "common/Common.hpp"

const char *poster_frag_code =
"//posf\n"
#if PLATFORM_WIN || PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"precision mediump float;  \n"

"out vec4 fColor;	\n"

"in vec2 vTexCoord;		\n"

// texture sampler
"uniform sampler2D image;	\n"

"void main()			\n"
"{						\n"
"	fColor = texture(image, vTexCoord);	\n"
"}						\n";

#endif
