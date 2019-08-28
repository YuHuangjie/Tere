#ifndef RENDER_FRAG_H
#define RENDER_FRAG_H

#include "Platform.h"
#include "Const.h"

#if MAX_NUM_INTERP > 20
#error MAX_NUM_INTERP must not be larger than 20
#endif

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

"const int MAX_NUM_INTERP = "
STR_MAX_NUM_INTERP(MAX_NUM_INTERP)"; \n"
"in highp vec4 vertex_location;   \n"
"in highp float[MAX_NUM_INTERP] depthNoOccul; \n"
"in highp vec3 vColor; \n"

"uniform highp int[MAX_NUM_INTERP] interpIndices; \n"
"uniform highp float[MAX_NUM_INTERP] interpWeights; \n"
"uniform highp int nInterps; \n"
"uniform highp sampler2D ref_cam_VP;\n"
"uniform highp sampler2D ref_cam_V;\n"
"uniform int N_REF_CAMERAS;\n"
"uniform mediump sampler2D lightField[MAX_NUM_INTERP]; \n"
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
"	if (nInterps >= i) {	\\\n"
"		tex_coord = CalcTexCoordRoutine(interpIndices[i-1]);\\\n"
"		pixels[i-1] = texture(lightField[i-1], vec2(tex_coord.x, tex_coord.y)).rgba;\\\n"
"	} } while(false);	\n"

// help macros for various MAX_NUM_INTERP
"#define REPEAT_PROJECT1() { PROJECT(1); }\n"
"#define REPEAT_PROJECT2() { REPEAT_PROJECT1(); PROJECT(2); }\n"
"#define REPEAT_PROJECT3() { REPEAT_PROJECT2(); PROJECT(3); }\n"
"#define REPEAT_PROJECT4() { REPEAT_PROJECT3(); PROJECT(4); }\n"
"#define REPEAT_PROJECT5() { REPEAT_PROJECT4(); PROJECT(5); }\n"
"#define REPEAT_PROJECT6() { REPEAT_PROJECT5(); PROJECT(6); }\n"
"#define REPEAT_PROJECT7() { REPEAT_PROJECT6(); PROJECT(7); }\n"
"#define REPEAT_PROJECT8() { REPEAT_PROJECT7(); PROJECT(8); }\n"
"#define REPEAT_PROJECT9() { REPEAT_PROJECT8(); PROJECT(9); }\n"
"#define REPEAT_PROJECT10() { REPEAT_PROJECT9(); PROJECT(10); }\n"
"#define REPEAT_PROJECT11() { REPEAT_PROJECT10(); PROJECT(11); }\n"
"#define REPEAT_PROJECT12() { REPEAT_PROJECT11(); PROJECT(12); }\n"
"#define REPEAT_PROJECT13() { REPEAT_PROJECT12(); PROJECT(13); }\n"
"#define REPEAT_PROJECT14() { REPEAT_PROJECT13(); PROJECT(14); }\n"
"#define REPEAT_PROJECT15() { REPEAT_PROJECT14(); PROJECT(15); }\n"
"#define REPEAT_PROJECT16() { REPEAT_PROJECT15(); PROJECT(16); }\n"
"#define REPEAT_PROJECT17() { REPEAT_PROJECT16(); PROJECT(17); }\n"
"#define REPEAT_PROJECT18() { REPEAT_PROJECT17(); PROJECT(18); }\n"
"#define REPEAT_PROJECT19() { REPEAT_PROJECT18(); PROJECT(19); }\n"
"#define REPEAT_PROJECT20() { REPEAT_PROJECT19(); PROJECT(20); }\n"
"#define REPEAT_PROJECT() REPEAT_PROJECT"
STR_MAX_NUM_INTERP(MAX_NUM_INTERP)"()\n"

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
"	vec4[MAX_NUM_INTERP] pixels;	\n"

// fetch projected pixels
"	REPEAT_PROJECT();\n"

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
