const char *depth_fragment_code =
"#version 300 es\n"
"precision highp float;\n"
"precision highp int;\n"

"out vec4 color;\n"
"in vec3 vertex_position;\n"

"uniform mat4 VP;\n"
"uniform float near;\n"
"uniform float far;\n"
"uniform sampler2D RGB;\n"

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
"	color = vec4(texture(RGB, tex_coord).rgb, depth);\n"
"}\n";
