#version 300 es
precision highp float;
precision highp int;

out vec4 color;
in vec3 vertex_position;

uniform mat4 VP;
uniform float near;
uniform float far;
uniform sampler2D RGB;

float LinearizeDepth(float depth) 
{ 
	float z = depth * 2.0 - 1.0; 
	// Back to NDC 
	return (2.0 * near * far) / (far + near - z * (far - near)); 
} 

void main()
{
	// equally divide the length between near and far (256 pieces)
	float depth = (LinearizeDepth(gl_FragCoord.z) - near) / (far - near);

	vec4 ndc = VP * vec4(vertex_position, 1.0);
	ndc = ndc / ndc.w;
	vec2 tex_coord = (ndc.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);
	color = vec4(texture(RGB, tex_coord).rgb, depth);
}
