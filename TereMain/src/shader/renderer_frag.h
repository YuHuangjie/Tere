#ifndef RENDER_FRAG_H
#define RENDER_FRAG_H

#include "common/Common.hpp"

const char *renderer_fragment_coder =
"//renf\n"
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

"vec4 missColor = vec4(255, 87, 155, 255) / 255.0;\n"

// calculate projected (u,v) of fragment
"vec2 CalcTexCoordRoutine(int cam_id) \n"
"{\n"
"	vec4 ndc_coord = mat4(texture(ref_cam_VP, vec2(float(8*cam_id + 1) / float(8*N_REF_CAMERAS), 0.0)), \n"
"		texture(ref_cam_VP, vec2(float(8*cam_id + 3) / float(8*N_REF_CAMERAS), 0.0)),\n"
"		texture(ref_cam_VP, vec2(float(8*cam_id + 5) / float(8*N_REF_CAMERAS), 0.0)),\n"
"		texture(ref_cam_VP, vec2(float(8*cam_id + 7) / float(8*N_REF_CAMERAS), 0.0))) * vertex_location;\n"
"	ndc_coord /= ndc_coord.w;\n"
"	vec2 tex_coord = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"	return tex_coord;\n"
"}\n"

"bool DepthTest(float pixelDepth, float depthNoOccul, float EPS) \n"
"{\n"
"	return (pixelDepth > 0.f && abs(depthNoOccul - pixelDepth) <= EPS);\n"
"}\n"


/******************************************************
* Do view-dependent texture blending. The number of reference cameras for
* blending is nInterps. Ref indices are stored in interpIndices. Blending
* weights are stored in interpWeights.
* The blending function is given as:
*   C = \frac{\sum_{i} W_i*(R_i, G_i, B_i, 1)*A_i}{\sum_{i} W_i}
******************************************************/
"void main()\n"
"{\n"
"	float	EPS				= 1.5 / 255.0;\n"		// depth test threshold
"	float	total_weight	= 0.0;\n"
"   float	weight			= 0.0f;    \n"
"   vec2	tex_coord		= vec2(0.0);  \n"
"	float	pixel_alpha		= 0.f; \n"
"	color					= vec4(0.0);      \n"
"	vec4[12] pixels;	\n"

// fetch projected pixels
"	if (nInterps >= 1) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[0]);\n"
"		pixels[0] = texture(lightField[0], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 2) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[1]);\n"
"		pixels[1] = texture(lightField[1], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 3) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[2]);\n"
"		pixels[2] = texture(lightField[2], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 4) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[3]);\n"
"		pixels[3] = texture(lightField[3], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 5) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[4]);\n"
"		pixels[4] = texture(lightField[4], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 6) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[5]);\n"
"		pixels[5] = texture(lightField[5], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 7) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[6]);\n"
"		pixels[6] = texture(lightField[6], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 8) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[7]);\n"
"		pixels[7] = texture(lightField[7], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 9) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[8]);\n"
"		pixels[8] = texture(lightField[8], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 10) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[9]);\n"
"		pixels[9] = texture(lightField[9], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 11) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[10]);\n"
"		pixels[10] = texture(lightField[10], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"
"	if (nInterps >= 12) {\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[11]);\n"
"		pixels[11] = texture(lightField[11], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"	}\n"

// Blend reference pixels
"	for (int i = 0; i != nInterps; ++i) {\n"
"		float _EPS = EPS * (1.f + float(i / 3));	\n"	// increment EPS gradually
"       weight = interpWeights[i]; \n"
"		weight *= float(DepthTest(pixels[i].w, depthNoOccul[i], _EPS));	\n"	// false is 0
//   alpha consumes 2 least significant bits in red channel
"		pixel_alpha = float((int(pixels[i].r * 255.0) & 0xC0) >> 6) / 3.f;	\n"
"       pixels[i].r = float((int(pixels[i].r * 255.0) & 0x3F) << 2) / 255.0f;   \n"
"		total_weight += weight; \n"
"		color.rgb += weight * pixels[i].rgb * pixel_alpha;  \n"
"		color.a += weight * pixel_alpha;	\n"
"	}\n"

// normalize final color or assign vertex(missing) color if the sum of weights
// are 0
"	if (total_weight > 0.0f) {\n"
"       color = color / total_weight;\n"
"	}\n"
"   else {\n"
// 3 strategies to handle miss rendered fragment
//      1. assign a hardcoded missing color (for debugging mainly)
//      2. assign vertex color
//      3. considered transparent (use background color)
// "		color = missColor; \n"
// "		color.xyz = vColor; \n"
"		discard; \n"
"	}\n"

"	//color = vec4(0, 0, 0, 1);\n"
"   //color.xy = tex_coord; \n"
"	//color.x = pixels[0].w;	\n"
"	//color.x = depthNoOccul[0];\n"
"}\n";

#endif
