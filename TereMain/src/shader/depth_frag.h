#ifndef DEPTH_FRAG_H
#define DEPTH_FRAG_H

#include "common/Common.hpp"

const char *depth_fragment_code =
"//depf\n"
#if PLATFORM_WIN || PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif
"precision highp float;\n"
"precision highp int;\n"

"out vec4 color;\n"
"in vec3 vertex_position;\n"
"in float vDepth;\n"

"uniform mat4 VP;\n"
"uniform float near;\n"
"uniform float far;\n"
"uniform sampler2D RGBA;\n"

"float LinearizeDepth(float depth)\n"
"{\n"
"	float z = depth * 2.0 - 1.0;\n"
// Back to NDC 
"	return (2.0 * near * far) / (far + near - z * (far - near));\n"
"}\n"

"void main()\n"
"{\n"
// equally divide the length between near and far (256 pieces)
"	float depth = (LinearizeDepth(gl_FragCoord.z) - near) / (far - near);\n"

"	vec4 ndc = VP * vec4(vertex_position, 1.0);\n"
"	ndc = ndc / ndc.w;\n"
"	vec2 tex_coord = (ndc.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"	vec4 rgba = texture(RGBA, tex_coord).rgba;	\n"
"	color = vec4(rgba.rgb, depth);\n"
// encode alpha
"	color.r = float((int(color.r * 255.0)&0xFC) | (int(rgba.a * 255.0)>>6)) / 255.0;\n"
"}\n";

#endif
