#ifndef TEXTURE_FRAG_H
#define TEXTURE_FRAG_H

#include "common/Common.hpp"

const char *fuser_frag_code =
"//fusfv\n"
#if PLATFORM_WIN || PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"out vec4 fColor;	\n"

"in vec2 vTexCoord;		\n"

"uniform float bgR;		\n"
"uniform float bgG;		\n"
"uniform float bgB;		\n"

// texture sampler
"uniform sampler2D fgTexture;	\n"

"void main()			\n"
"{						\n"
"	vec4 _fgColor = texture(fgTexture, vTexCoord);	\n"
"	vec4 _bgColor = vec4(bgR, bgG, bgB, 1.0f);	\n"
"	fColor = mix(_bgColor, _fgColor, _fgColor.a);	\n"
"}						\n";

#endif
