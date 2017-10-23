const char *renderer_fragment_coder = 
"#version 300 es\n"
"precision highp float;\n"
"precision highp int;\n"
"precision highp sampler2D;\n"

// quad texture test
"in highp vec2 my_tex_coord;\n"

"in highp float[4] depth_no_occul;\n"
"in highp float[4] T_depth_no_occul;\n"
"in highp float[4] T2_depth_no_occul;\n"
"in highp vec4 vertex_location;\n"

"uniform highp ivec4 CAMERAS;\n"			// closest reference cameras around eye
"uniform highp ivec4 INDIRECT_CAMERAS;\n"	// reserved cameras for invisible area
"uniform highp ivec4 DINDIRECT_CAMERAS;\n"	// second level reserved cameras for invisible area
"uniform highp vec4 WEIGHTS;\n"				// weights assigned to CAMERAS
"uniform highp vec4 INDIRECT_WEIGHTS;\n"
"uniform highp vec4 DINDIRECT_WEIGHTS;\n"
"uniform highp sampler2D ref_cam_VP;\n"
"uniform highp sampler2D ref_cam_V;\n"
"uniform int N_REF_CAMERAS;\n"

"uniform highp sampler2D light_field[4];\n"		// textures corresponding with CAMERAS
"uniform highp sampler2D T_light_field[4];\n"	// textures corresponding with INDIRECT_CAMERAS
"uniform highp sampler2D T2_light_field[4];\n"	// textures corresponding with DINDIRECT_CAMERAS
"uniform highp float near;\n"
"uniform highp float far;\n"

"out highp vec4 color;\n"

"vec4 default_color = vec4(255, 87, 155, 1) / 255.0;\n"
"vec4 invisible_color = vec4(1, 1, 1, 1);\n"

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

"	vec4 weights = WEIGHTS;\n"
"	vec4[4] pix = vec4[4](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));\n"
"	vec2 tex_coord[4] = vec2[4](vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0));\n"

"	vec4 T_weights = INDIRECT_WEIGHTS;	// equal weights for indirect cameras\n"
"	vec4 T_pix[4] = vec4[4](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));\n"
"	vec2 T_tex_coord[4] = vec2[4](vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0));\n"

"	vec4 T2_weights = DINDIRECT_WEIGHTS;	// // equal weights for double-indirect cameras\n"
"	vec4 T2_pix[4] = vec4[4](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));\n"
"	vec2 T2_tex_coord[4] = vec2[4](vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0));\n"


/* Render from direct cameras */
/* depth tests */
"	for (int i = 0; i != 4; ++i) {\n"
"		cam_id = CAMERAS[i];\n"
"		ndc_coord = mat4(texture(ref_cam_VP, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"			texture(ref_cam_VP, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"		ndc_coord /= ndc_coord.w;\n"
"		tex_coord[i] = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"	}\n"
"	pix[0] = texture(light_field[0], vec2(tex_coord[0].x, tex_coord[0].y)).rgba;\n"
"	pix[1] = texture(light_field[1], vec2(tex_coord[1].x, tex_coord[1].y)).rgba;\n"
"	pix[2] = texture(light_field[2], vec2(tex_coord[2].x, tex_coord[2].y)).rgba;\n"
"	pix[3] = texture(light_field[3], vec2(tex_coord[3].x, tex_coord[3].y)).rgba;\n"
"	if (pix[0].w <= 0.f || abs(depth_no_occul[0] - pix[0].w) > EPS || tex_coord[0].x <= episilon || tex_coord[0].x >= 1.0 - episilon || tex_coord[0].y <= episilon || tex_coord[0].y >= 1.0 - episilon) weights[0] = 0.0;\n"
"	if (pix[1].w <= 0.f || abs(depth_no_occul[1] - pix[1].w) > EPS || tex_coord[1].x <= episilon || tex_coord[1].x >= 1.0 - episilon || tex_coord[1].y <= episilon || tex_coord[1].y >= 1.0 - episilon) weights[1] = 0.0;\n"
"	if (pix[2].w <= 0.f || abs(depth_no_occul[2] - pix[2].w) > EPS || tex_coord[2].x <= episilon || tex_coord[2].x >= 1.0 - episilon || tex_coord[2].y <= episilon || tex_coord[2].y >= 1.0 - episilon) weights[2] = 0.0;\n"
"	if (pix[3].w <= 0.f || abs(depth_no_occul[3] - pix[3].w) > EPS || tex_coord[3].x <= episilon || tex_coord[3].x >= 1.0 - episilon || tex_coord[3].y <= episilon || tex_coord[3].y >= 1.0 - episilon) weights[3] = 0.0;\n"

"	total_weight = weights[0] + weights[1] + weights[2] + weights[3];\n"

/* blending */
"	if (total_weight > 0.0) {\n"
"		for (int i = 0; i != 4; ++i) weights[i] = weights[i] / total_weight;\n"
"		color = vec4(0.0);\n"
"		for (int i = 0; i != 4; ++i) color += weights[i] * pix[i];\n"
"	}\n"
"	else {\n"
/* Render from indirect cameras */
"		for (int i = 0; i != 4; ++i) {\n"
"			cam_id = INDIRECT_CAMERAS[i];\n"
"			ndc_coord = mat4(texture(ref_cam_VP, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"				texture(ref_cam_VP, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"				texture(ref_cam_VP, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"				texture(ref_cam_VP, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"			ndc_coord /= ndc_coord.w;\n"
"			T_tex_coord[i] = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"		}\n"
"		T_pix[0] = texture(T_light_field[0], vec2(T_tex_coord[0].x, T_tex_coord[0].y)).rgba;\n"
"		T_pix[1] = texture(T_light_field[1], vec2(T_tex_coord[1].x, T_tex_coord[1].y)).rgba;\n"
"		T_pix[2] = texture(T_light_field[2], vec2(T_tex_coord[2].x, T_tex_coord[2].y)).rgba;\n"
"		T_pix[3] = texture(T_light_field[3], vec2(T_tex_coord[3].x, T_tex_coord[3].y)).rgba;\n"
"		if (T_pix[0].w <= 0.f || abs(T_depth_no_occul[0] - T_pix[0].w) > EPS || T_tex_coord[0].x <= episilon || T_tex_coord[0].x >= 1.0 - episilon || T_tex_coord[0].y <= episilon || T_tex_coord[0].y >= 1.0 - episilon) T_weights[0] = 0.0;\n"
"		if (T_pix[1].w <= 0.f || abs(T_depth_no_occul[1] - T_pix[1].w) > EPS || T_tex_coord[1].x <= episilon || T_tex_coord[1].x >= 1.0 - episilon || T_tex_coord[1].y <= episilon || T_tex_coord[1].y >= 1.0 - episilon) T_weights[1] = 0.0;\n"
"		if (T_pix[2].w <= 0.f || abs(T_depth_no_occul[2] - T_pix[2].w) > EPS || T_tex_coord[2].x <= episilon || T_tex_coord[2].x >= 1.0 - episilon || T_tex_coord[2].y <= episilon || T_tex_coord[2].y >= 1.0 - episilon) T_weights[2] = 0.0;\n"
"		if (T_pix[3].w <= 0.f || abs(T_depth_no_occul[3] - T_pix[3].w) > EPS || T_tex_coord[3].x <= episilon || T_tex_coord[3].x >= 1.0 - episilon || T_tex_coord[3].y <= episilon || T_tex_coord[3].y >= 1.0 - episilon) T_weights[3] = 0.0;\n"

"		total_weight = T_weights[0] + T_weights[1] + T_weights[2] + T_weights[3];\n"

"		if (total_weight > 0.0) {\n"
"			for (int i = 0; i != 4; ++i) T_weights[i] = T_weights[i] / total_weight;\n"
"			color = vec4(0.0);\n"
"			for (int i = 0; i != 4; ++i) color += T_weights[i] * T_pix[i];\n"
"		}\n"
"		else {\n"
/* Render from double indirect cameras */
"			for (int i = 0; i != 4; ++i) {\n"
"				cam_id = DINDIRECT_CAMERAS[i];\n"
"				ndc_coord = mat4(texture(ref_cam_VP, vec2(float(4 * cam_id) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"					texture(ref_cam_VP, vec2(float(4 * cam_id + 1) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"					texture(ref_cam_VP, vec2(float(4 * cam_id + 2) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0)),\n"
"					texture(ref_cam_VP, vec2(float(4 * cam_id + 3) / float(4 * N_REF_CAMERAS + 1 - 1), 0.0))) * vertex_location;\n"
"				ndc_coord /= ndc_coord.w;\n"
"				T2_tex_coord[i] = (ndc_coord.xy + vec2(1.0, 1.0)) / vec2(2.0, 2.0);\n"
"			}\n"
"			T2_pix[0] = texture(T2_light_field[0], vec2(T2_tex_coord[0].x, T2_tex_coord[0].y)).rgba;\n"
"			T2_pix[1] = texture(T2_light_field[1], vec2(T2_tex_coord[1].x, T2_tex_coord[1].y)).rgba;\n"
"			T2_pix[2] = texture(T2_light_field[2], vec2(T2_tex_coord[2].x, T2_tex_coord[2].y)).rgba;\n"
"			T2_pix[3] = texture(T2_light_field[3], vec2(T2_tex_coord[3].x, T2_tex_coord[3].y)).rgba;\n"
"			if (T2_pix[0].w <= 0.f || abs(T2_depth_no_occul[0] - T2_pix[0].w) > EPS) T2_weights[0] = 0.0;\n"
"			if (T2_pix[1].w <= 0.f || abs(T2_depth_no_occul[1] - T2_pix[1].w) > EPS) T2_weights[1] = 0.0;\n"
"			if (T2_pix[2].w <= 0.f || abs(T2_depth_no_occul[2] - T2_pix[2].w) > EPS) T2_weights[2] = 0.0;\n"
"			if (T2_pix[3].w <= 0.f || abs(T2_depth_no_occul[3] - T2_pix[3].w) > EPS) T2_weights[3] = 0.0;\n"

"			total_weight = T2_weights[0] + T2_weights[1] + T2_weights[2] + T2_weights[3];\n"
"			for (int i = 0; i != 4; ++i) T2_weights[i] = T2_weights[i] / total_weight;\n"
"			color = vec4(0.0);\n"
"			for (int i = 0; i != 4; ++i) color += T2_weights[i] * T2_pix[i];\n"
"		}\n"
"	}\n"

"	// 	if (CAMERAS[0] == -1 && CAMERAS[1] == -1 && CAMERAS[2] == -1 && CAMERAS[3] == -1) {\n"
"	// 		color = invisible_color;\n"
"	// 	}\n"
"	// 	else {\n"
"	// 		color = vec4(0, 0, 0, 1);\n"
"	//		color.x = abs(depth_no_occul[2] - pix[2].w);\n"

"	// 		//color = texture(depth[0], my_tex_coord);\n"
"	// 	}\n"
"}\n";