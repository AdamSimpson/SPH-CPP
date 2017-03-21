#version 150 core
in vec3 position;
in vec3 normal;
in vec2 tex_coord;

out vec4 camera_space_frag_pos;
out vec4 camera_space_normal;
out vec2 frag_tex_coord;

layout(std140) uniform view_matrices
{
    mat4 world_to_camera_matrix;
    mat4 camera_to_clip;
};

void main() {
    camera_space_frag_pos = world_to_camera_matrix*vec4(position, 1.0);
    camera_space_normal = world_to_camera_matrix*vec4(normal, 0.0);
    gl_Position = camera_to_clip*camera_space_frag_pos;

    frag_tex_coord = tex_coord;
}
