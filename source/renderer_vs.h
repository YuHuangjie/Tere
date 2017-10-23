const char *renderer_vertex_code =
"#version 300 es\n"

"layout(location = 0) in vec3 vertex_position_modelspace; \n"	// vertex location in model space
"layout(location = 1) in vec2 quad_tex_coord; \n"

// View Projection matrix of rendering camera
"uniform mat4 VP;\n"
// ID of closest reference cameras around eye
"uniform ivec4 CAMERAS; \n"
// INDIRECT_CAMERAS are cameras around CAMERAS for rendering area which are invisible from CAMERAS
"uniform highp ivec4 INDIRECT_CAMERAS; \n"
// DINDIRECT_CAMERAS are cameras around INDIRECT_CAMERAS for rendering area which are invisible from INDIRECT_CAMERAS
"uniform highp ivec4 DINDIRECT_CAMERAS; \n"
// reference cameras' VP matices (each matrix of the size [4*N_REF_CAMERAS, 4])
"uniform highp sampler2D ref_cam_VP; \n"
// reference cameras' V matices (each matrix of the size [4*N_REF_CAMERAS, 4])       
"uniform highp sampler2D ref_cam_V; \n"
// number of reference cameras       
"uniform int N_REF_CAMERAS;\n"

"out float[4] depth_no_occul;	\n"// depth of vertex in camera's coordinate system (CAMERAS)
"out float[4] T_depth_no_occul;	\n"// depth of vertex in camera's coordinate system (INDIRECT_CAMERAS)
"out float[4] T2_depth_no_occul;	\n"// depth of vertex in camera's coordinate system (DINDIRECT_CAMERAS)
"out vec4 vertex_location;\n"

// quad texture test
"out vec2 my_tex_coord;\n"

"uniform float near;\n"	// near plane
"uniform float far;\n"	// far plane

"void main()\n"
"{\n"
"	vertex_location = vec4(vertex_position_modelspace, 1);\n"
"	gl_Position = VP * vertex_location; \n"

// declarations
"	vec4 vertex_in_camera;\n"
"	vec4 ndc_coord; \n"
"	int cam_id;\n"

/* direct */
"	for (int i = 0; i != 4; ++i) {\n"
"		if (CAMERAS[i] == -1) continue;		\n"// invalid camera index
"		cam_id = CAMERAS[i];\n"
"		vertex_in_camera = mat4(texture(ref_cam_V, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"		vertex_in_camera /= vertex_in_camera.w;\n"
"		depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);\n"
"	}\n"
/* indirect */
"	for (int i = 0; i != 4; ++i) {\n"
"		if (INDIRECT_CAMERAS[i] == -1) continue; \n"
"		cam_id = INDIRECT_CAMERAS[i];\n"
"		vertex_in_camera = mat4(texture(ref_cam_V, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)), \n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)), \n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"		vertex_in_camera /= vertex_in_camera.w; \n"
"		T_depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);\n"
"	}\n"
/* double indirect */
"for (int i = 0; i != 4; ++i) {	\n"
"		if (DINDIRECT_CAMERAS[i] == -1) continue;\n"
"		cam_id = DINDIRECT_CAMERAS[i];\n"
"		vertex_in_camera = mat4(texture(ref_cam_V, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"		vertex_in_camera /= vertex_in_camera.w;\n"
"		T2_depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);\n"
"	}\n"

// test drawing
//gl_Position = vec4(vertex_position_modelspace, 1);
//my_tex_coord = quad_tex_coord;
"}\n";