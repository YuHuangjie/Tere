#ifndef RENDER_VS_H
#define RENDER_VS_H

#include "Platform.h"
#include "Const.h"

const char *SCENE_VS =
"//snvs\n"
#if defined PLATFORM_WIN || defined PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif

"layout(location = 0) in vec3 vertex_position_modelspace; \n"	// vertex location in model space
"layout(location = 1) in vec3 color; \n"	// vertex color

"const int MAX_NUM_INTERP = "
STR_MAX_NUM_INTERP(MAX_NUM_INTERP)"; \n"
// View Projection matrix of rendering camera
"uniform mat4 VP;\n"
// reference cameras' VP matices (each matrix of the size [4*N_REF_CAMERAS, 4])
"uniform highp sampler2D ref_cam_VP; \n"
// reference cameras' V matices (each matrix of the size [4*N_REF_CAMERAS, 4])       
"uniform highp sampler2D ref_cam_V; \n"
// reference cameras' indices
"uniform highp int[MAX_NUM_INTERP] interpIndices; \n"
"uniform highp int nInterps; \n"
// number of reference cameras       
"uniform int N_REF_CAMERAS;\n"

"out float[MAX_NUM_INTERP] depthNoOccul;    \n"
"out vec4 vertex_location;\n"
"out vec3 vColor;\n"

// quad texture test
"out vec2 my_tex_coord;\n"

"uniform float near;\n"	// near plane
"uniform float far;\n"	// far plane

"void main()\n"
"{\n"
"	vertex_location = vec4(vertex_position_modelspace, 1);\n"
"	gl_Position = VP * vertex_location; \n"
"   vColor = color; \n"

// declarations
"	vec4 vertex_in_camera;\n"
"	vec4 ndc_coord; \n"
"	int cam_id;\n"

"for (int i = 0; i != nInterps; ++i) {	\n"
"		cam_id = interpIndices[i];\n"
"		vertex_in_camera = mat4(texture(ref_cam_V, vec2(float(8*cam_id + 1) / float(8*N_REF_CAMERAS), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(8*cam_id + 3) / float(8*N_REF_CAMERAS), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(8*cam_id + 5) / float(8*N_REF_CAMERAS), 0.0)),\n"
"			texture(ref_cam_V, vec2(float(8*cam_id + 7) / float(8*N_REF_CAMERAS), 0.0))) * vertex_location;\n"
"		vertex_in_camera /= vertex_in_camera.w;\n"
"		depthNoOccul[i] = (-vertex_in_camera.z - near) / (far - near);\n"
"	}\n"


// test drawing
//gl_Position = vec4(vertex_position_modelspace, 1);
//my_tex_coord = quad_tex_coord;
"}\n";

#endif
