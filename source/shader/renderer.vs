#version 300 es

layout(location = 0) in vec3 vertex_position_modelspace;	// vertex location in model space
layout(location = 1) in vec2 quad_tex_coord;

// View Projection matrix of rendering camera
uniform mat4 VP;				
// ID of closest reference cameras around eye
uniform ivec4 CAMERAS;			
// INDIRECT_CAMERAS are cameras around CAMERAS for rendering area which are invisible from CAMERAS
uniform highp ivec4 INDIRECT_CAMERAS;	
// DINDIRECT_CAMERAS are cameras around INDIRECT_CAMERAS for rendering area which are invisible from INDIRECT_CAMERAS
uniform highp ivec4 DINDIRECT_CAMERAS;	
// reference cameras' VP matices (each matrix of the size [4*N_REF_CAMERAS, 4])
uniform highp sampler2D ref_cam_VP;
// reference cameras' V matices (each matrix of the size [4*N_REF_CAMERAS, 4])       
uniform highp sampler2D ref_cam_V; 
// number of reference cameras       
uniform int N_REF_CAMERAS;			

out float[4] depth_no_occul;	// depth of vertex in camera's coordinate system (CAMERAS)
out float[4] T_depth_no_occul;	// depth of vertex in camera's coordinate system (INDIRECT_CAMERAS)
out float[4] T2_depth_no_occul;	// depth of vertex in camera's coordinate system (DINDIRECT_CAMERAS)
out vec4 vertex_location;

// quad texture test
out vec2 my_tex_coord;

uniform float near;	// near plane
uniform float far;	// far plane

void main()
{
	vertex_location = vec4(vertex_position_modelspace, 1);
	gl_Position = VP * vertex_location;

	// declarations
	vec4 vertex_in_camera;
	vec4 ndc_coord;
    int cam_id;

/* direct */
	for (int i = 0; i != 4; ++i) {
		if (CAMERAS[i] == -1) continue;		// invalid camera index
		cam_id = CAMERAS[i];
		vertex_in_camera = mat4( texture(ref_cam_V, vec2(float(4*cam_id) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+1) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+2) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+3) / float(4*N_REF_CAMERAS+1-1), 0.0)) ) * vertex_location;		
		vertex_in_camera /= vertex_in_camera.w;
		depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);
	}
/* indirect */
	for (int i = 0; i != 4; ++i) {
		if (INDIRECT_CAMERAS[i] == -1) continue;
		cam_id = INDIRECT_CAMERAS[i];
		vertex_in_camera = mat4( texture(ref_cam_V, vec2(float(4*cam_id) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+1) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+2) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+3) / float(4*N_REF_CAMERAS+1-1), 0.0)) ) * vertex_location;
		vertex_in_camera /= vertex_in_camera.w;
		T_depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);
	}
/* double indirect */
	for (int i = 0; i != 4; ++i) {
		if (DINDIRECT_CAMERAS[i] == -1) continue;
		cam_id = DINDIRECT_CAMERAS[i];
		vertex_in_camera = mat4( texture(ref_cam_V, vec2(float(4*cam_id) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+1) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+2) / float(4*N_REF_CAMERAS+1-1), 0.0)),
                     texture(ref_cam_V, vec2(float(4*cam_id+3) / float(4*N_REF_CAMERAS+1-1), 0.0)) ) * vertex_location;
		vertex_in_camera /= vertex_in_camera.w;
		T2_depth_no_occul[i] = (-vertex_in_camera.z - near) / (far - near);
	}

	// test drawing
	//gl_Position = vec4(vertex_position_modelspace, 1);
	//my_tex_coord = quad_tex_coord;
}