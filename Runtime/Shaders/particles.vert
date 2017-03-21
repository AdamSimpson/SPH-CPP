#version 150 core
in vec3 position;
in vec3 color;

layout(std140) uniform view_matrices
{
    mat4 world_to_camera_matrix;
    mat4 camera_to_clip;
};

out vertex_data
{
    vec3 camera_sphere_position;
    vec3 sphere_color;
} out_data;

void main() {
   out_data.camera_sphere_position = (world_to_camera_matrix*vec4(position, 1.0)).xyz;
   out_data.sphere_color = color;
}
