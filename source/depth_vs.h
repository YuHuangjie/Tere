const char *depth_vertex_code =
"#version 300 es\n"

"layout(location = 0) in vec3 vertexPosition_modelspace;\n"

"uniform mat4 VP;\n"

"out vec3 vertex_position;\n"

"void main()\n"
"{\n"
"	gl_Position = VP * vec4(vertexPosition_modelspace, 1.0);\n"
"	vertex_position = vertexPosition_modelspace;\n"
"}\n";
