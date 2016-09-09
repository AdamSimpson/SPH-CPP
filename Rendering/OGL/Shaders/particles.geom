#version 330

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

layout(std140) uniform view_matrices
{
    mat4 world_to_camera_matrix;
    mat4 camera_to_clip;
};

uniform float sphere_radius;

in vertex_data
{
    vec3 camera_sphere_position;
    vec3 sphere_color;
} vert[];

out frag_data
{
    flat vec3 camera_sphere_position;
    flat vec3 sphere_color;
    smooth vec2 mapping;
};

const float g_box_correction = 1.5;

void main()
{
	vec4 camera_corner_position;
	//Bottom-left
	mapping = vec2(-1.0, -1.0) * g_box_correction;
	camera_sphere_position = vec3(vert[0].camera_sphere_position);
	sphere_color = vert[0].sphere_color;
	camera_corner_position = vec4(vert[0].camera_sphere_position, 1.0);
	camera_corner_position.xy += vec2(-sphere_radius, -sphere_radius) * g_box_correction;
	gl_Position = camera_to_clip * camera_corner_position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Top-left
	mapping = vec2(-1.0, 1.0) * g_box_correction;
	camera_sphere_position = vec3(vert[0].camera_sphere_position);
	sphere_color = vert[0].sphere_color;
	camera_corner_position = vec4(vert[0].camera_sphere_position, 1.0);
	camera_corner_position.xy += vec2(-sphere_radius, sphere_radius) * g_box_correction;
	gl_Position = camera_to_clip * camera_corner_position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Bottom-right
	mapping = vec2(1.0, -1.0) * g_box_correction;
	camera_sphere_position = vec3(vert[0].camera_sphere_position);
	sphere_color = vert[0].sphere_color;
	camera_corner_position = vec4(vert[0].camera_sphere_position, 1.0);
	camera_corner_position.xy += vec2(sphere_radius, -sphere_radius) * g_box_correction;
	gl_Position = camera_to_clip * camera_corner_position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Top-right
	mapping = vec2(1.0, 1.0) * g_box_correction;
	camera_sphere_position = vec3(vert[0].camera_sphere_position);
	sphere_color = vert[0].sphere_color;
	camera_corner_position = vec4(vert[0].camera_sphere_position, 1.0);
	camera_corner_position.xy += vec2(sphere_radius, sphere_radius) * g_box_correction;
	gl_Position = camera_to_clip * camera_corner_position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();
}
