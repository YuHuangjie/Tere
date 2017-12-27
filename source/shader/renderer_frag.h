const char *renderer_fragment_coder =
"#version 300 es\n"
"precision highp float;\n"
"precision highp int;\n"

// quad texture test
"in highp vec2 my_tex_coord;\n"

"in highp vec4 vertex_location;   \n"
"in highp float[12] depthNoOccul; \n"

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

/******************************************************
* At first, we search four cameras surrounding virtual eye and perform light field rendering.
* The idea is when four cameras are not enough to render a object (i.e. there're holes),
* we search for another four cameras around last four (the indirect cameras) and fill holes using them.
* If still there're holes, we search for four more cameras (the double indirect cameras) for filling holes.
******************************************************/
"void main()\n"
"{\n"
"	const float EPS = 1.5 / 255.0;\n"		// depth test threshold
"	const float episilon = 1e-2;\n"
"	float total_weight = 0.0;\n"
"	vec4 ndc_coord;\n"
"	int cam_id;\n"

"	const int MAX_INTERP = 4; \n"
"   int interpCount = 0; \n"

"	float weight = 0.0f;    \n"
"	vec4 pixel = vec4(0.0);   \n"
"	vec2 tex_coord = vec2(0.0);  \n"
"	color = vec4(0.0);      \n"

// Render from interpolating cameras
// depth tests
"	for (int i = 0; i != nInterps; ++i) {\n"
"		cam_id = interpIndices[i];\n"
"		ndc_coord = mat4(texture(ref_cam_VP, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"		ndc_coord /= ndc_coord.w;\n"
"		tex_coord = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"		pixel = texture(lightField[i], vec2(tex_coord.x, tex_coord.y)).rgba;\n"
"       weight = interpWeights[i]; \n"
"		if (pixel.w <= 0.f || abs(depthNoOccul[i] - pixel.w) > EPS) weight = 0.f; \n"
"		total_weight += weight; \n"
"		color += weight * pixel;  \n"

"		interpCount++; \n"
"       if (interpCount == MAX_INTERP) {\n"
"           break;\n"
"		}\n"
"	}\n"

"	if (total_weight > 0.0f) {\n"
"       color = color / total_weight;\n"
"	}\n"
"   else {\n"
"		color = missColor; \n"
"	}\n"


"	//color = vec4(0, 0, 0, 1);\n"
"	//color = debugWeight * debugPixel;\n"
"	//color.x = abs(depthNoOccul[11] - pixel.w);\n"
"}\n";