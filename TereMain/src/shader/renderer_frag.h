#ifndef RENDER_FRAG_H
#define RENDER_FRAG_H

#include "Platform.h"

const char *SCENE_FS =
"//snfs\n"
#if defined PLATFORM_WIN || defined PLATFORM_OSX
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

"#define PROJECT(i) do { \\\n"
"	if (nInterps >= i+1) {	\\\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[i]);\\\n"
"		pixels[i] = texture(lightField[i], vec2(tex_coord.x, tex_coord.y)).rgba;\\\n"
"	} } while(false);	\n"

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
"	PROJECT(0);\n"
"	PROJECT(1);\n"
"	PROJECT(2);\n"
"	PROJECT(3);\n"
"	PROJECT(4);\n"
"	PROJECT(5);\n"
"	PROJECT(6);\n"
"	PROJECT(7);\n"
"	PROJECT(8);\n"
"	PROJECT(9);\n"
"	PROJECT(10);\n"
"	PROJECT(11);\n"

// Blend reference pixels
"	for (int i = 0; i != nInterps; ++i) {\n"
"		float _EPS = EPS * (1.f + float(i / 3));	\n"	// increment EPS gradually
"       weight = interpWeights[i]; \n"
"		weight *= float(DepthTest(pixels[i].w, depthNoOccul[i], _EPS));	\n"	// false is 0
//   alpha consumes 2 least significant bits in red channel
"		pixel_alpha = float(int(pixels[i].r * 255.0) & 0x03) / 3.f;	\n"
"		total_weight += weight; \n"
"		color.rgb += weight * pixels[i].rgb * pixel_alpha;  \n"
"		color.a += weight * pixel_alpha;	\n"
"	}\n"

// normalize final color or assign vertex(missing) color if the sum of weights
// are 0
"	if (total_weight > 0.0f) {\n"
"       color = color / total_weight;\n"
		// light field is in BGR format
"		color.xz = color.zx;	\n"
"	}\n"
"   else {\n"
// 3 strategies to handle miss rendered fragment
//      1. assign a hardcoded missing color (for debugging mainly)
"		color = missColor; \n"
//      2. assign vertex color
// "	color.xyz = vColor; \n"
//      3. considered transparent (use background color)
//"		discard; \n"
"	}\n"

"	//color = vec4(0, 0, 0, 1);\n"
"   //color.xy = tex_coord; \n"
"	//color.x = pixels[0].w;	\n"
"	//color.x = depthNoOccul[0];\n"
"}\n";

#endif
