#ifndef DEPTH_VS_H
#define DEPTH_VS_H

#include "Platform.h"

const char *DEPTH_VS =
"//dpvs\n"
#if defined PLATFORM_WIN || defined PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"layout(location = 0) in vec3 vertexPosition_modelspace;\n"

"uniform mat4 VP;\n"

"out vec3 vertex_position;\n"

"void main()\n"
"{\n"
"	gl_Position = VP * vec4(vertexPosition_modelspace, 1.0);\n"
"	vertex_position = vertexPosition_modelspace;\n"
"}\n";

#endif
