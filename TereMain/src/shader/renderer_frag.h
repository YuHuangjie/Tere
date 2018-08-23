#ifndef RENDER_FRAG_H
#define RENDER_FRAG_H

#include "common/Common.hpp"

const char *renderer_fragment_coder =
#if PLATFORM_WIN || PLATFORM_OSX
"#version 330 \n"
#else
"#version 300 es\n"
#endif
"precision highp float;\n"
"precision highp int;\n"

// quad texture test
"in highp vec2 my_tex_coord;\n"

"in highp vec4 vertex_location;   \n"
"in highp float[12] depthNoOccul; \n"
"in highp vec3 vColor; \n"

"uniform highp int[12] interpIndices; \n"
"uniform highp float[12] interpWeights; \n"
"uniform highp int nInterps; \n"
"uniform highp sampler2D ref_cam_VP;\n"
"uniform highp sampler2D ref_cam_V;\n"
"uniform int N_REF_CAMERAS;\n"
"uniform mediump sampler2D lightField[12]; \n"
"uniform highp float near;\n"
"uniform highp float far;\n"

"out highp vec4 color;\n"

"vec4 missColor = vec4(255, 87, 155, 1) / 255.0;\n"

"vec2 CalcTexCoordRoutine(int cam_id) \n"
"{\n"
"	vec4 ndc_coord = mat4(texture(ref_cam_VP, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS - 1), 0.0)), \n"
"		texture(ref_cam_VP, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS - 1), 0.0)),\n"
"		texture(ref_cam_VP, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS - 1), 0.0)),\n"
"		texture(ref_cam_VP, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS - 1), 0.0))) * vertex_location;\n"
"	ndc_coord /= ndc_coord.w;\n"
"	vec2 tex_coord = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"	return tex_coord;\n"
"}\n"


/******************************************************
* At first, we search four cameras surrounding virtual eye and perform light field rendering.
* The idea is when four cameras are not enough to render a object (i.e. there're holes),
* we search for another four cameras around last four (the indirect cameras) and fill holes using them.
* If still there're holes, we search for four more cameras (the double indirect cameras) for filling holes.
******************************************************/
"void main()\n"
"{\n"
"	float EPS = 1.5 / 255.0;\n"		// depth test threshold
"	float total_weight = 0.0;\n"
"	int cam_id;\n"
"   float weight = 0.0f;    \n"
"   vec4 pixel = vec4(0.0);   \n"
"   vec2 tex_coord = vec2(0.0);  \n"

"	const int MAX_INTERP = 4; \n"
"   int interpCount = 0; \n"

"	color = vec4(0.0);      \n"

// Render from interpolating cameras
"	for (int i = 0; i != 1; ++i) {\n"
// # 0
"		if (nInterps < 1) { break; } \n"
"		cam_id = interpIndices[0];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"		pixel = texture(lightField[0], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[0]; \n"
"		if (pixel.w <= 0.f || abs(depthNoOccul[0] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"		total_weight += weight; \n"
"		color += weight * pixel;  \n"
"		interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 1
"		if (nInterps < 2) { break; } \n"
"        cam_id = interpIndices[1];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[1], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[1]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[1] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 2
"		if (nInterps < 3) { break; } \n"
"        cam_id = interpIndices[2];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[2], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[2]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[2] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 3
"		if (nInterps < 4) { break; } \n"
"        cam_id = interpIndices[3];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[3], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[3]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[3] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// Loose condition
"        EPS *= 2.0;\n"
// # 4
"		if (nInterps < 5) { break; } \n"
"        cam_id = interpIndices[4];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[4], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[4]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[4] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 5
"		if (nInterps < 6) { break; } \n"
"        cam_id = interpIndices[5];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[5], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[5]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[5] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 6
"		if (nInterps < 7) { break; } \n"
"        cam_id = interpIndices[6];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[6], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[6]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[6] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 7
"		if (nInterps < 8) { break; } \n"
"        cam_id = interpIndices[7];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[7], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[7]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[7] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// Loose condition
"        EPS *= 2.0;\n"
// # 8
"		if (nInterps < 9) { break; } \n"
"        cam_id = interpIndices[8];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[8], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[8]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[8] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 9
"		if (nInterps < 10) { break; } \n"
"        cam_id = interpIndices[9];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[9], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[9]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[9] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 10
"		if (nInterps < 11) { break; } \n"
"        cam_id = interpIndices[10];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[10], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[10]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[10] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
// # 11
"		if (nInterps < 12) { break; } \n"
"        cam_id = interpIndices[11];\n"
"		tex_coord = CalcTexCoordRoutine(cam_id);\n"
"        pixel = texture(lightField[11], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[11]; \n"
"        if (pixel.w <= 0.f || abs(depthNoOccul[11] - pixel.w) > EPS) { weight = 0.f; interpCount--; }\n"
"        total_weight += weight; \n"
"        color += weight * pixel;  \n"
"        interpCount++; \n"
"       if (interpCount == MAX_INTERP) { break; }\n"
"    }\n"

"	if (total_weight > 0.0f) {\n"
"       color = color / total_weight;\n"
"	}\n"
"   else {\n"
//"		color = missColor; \n"
"		color.xyz = vColor; \n"
"	}\n"

"	//color = vec4(0, 0, 0, 1);\n"
"   //color.xy = tex_coord; \n"
"	//color.x = abs(depthNoOccul[11] - pixel.w);\n"
"}\n";

#endif
