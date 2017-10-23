#version 300 es

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 VP;

out vec3 vertex_position;

void main()
{
	gl_Position = VP * vec4(vertexPosition_modelspace, 1.0);
	vertex_position = vertexPosition_modelspace;
}
