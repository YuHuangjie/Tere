#ifndef TEXTURE_FRAG_H
#define TEXTURE_FRAG_H

#include "Platform.h"

const char *fuser_frag_code =
"//fusfv\n"
#if defined PLATFORM_WIN || defined PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"precision mediump float;  \n"

"out vec4 fColor;	\n"

"in vec2 vTexCoord;		\n"

"uniform float bgR;		\n"
"uniform float bgG;		\n"
"uniform float bgB;		\n"
"uniform bool  monochromatic;   \n"

// texture sampler
"uniform sampler2D fgTexture;	\n"
"uniform sampler2D bgTexture;   \n"

"void main()			\n"
"{						\n"
"	vec4 _fgColor = texture(fgTexture, vTexCoord);	\n"
"   vec4 _bgColor = ( monochromatic ? vec4(bgR, bgG, bgB, 1.0f) : texture(bgTexture, vec2(vTexCoord.x, 1.f-vTexCoord.y)) );   \n"
"	fColor = mix(_bgColor, _fgColor, _fgColor.a);	\n"
"}						\n";

#endif
