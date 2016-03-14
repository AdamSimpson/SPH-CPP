#version 150 core
in vec2 coord;
in vec2 tex_coord;

uniform mat4 projection;

out vec2 frag_tex_coord;

void main() {
    frag_tex_coord = tex_coord;
    gl_Position = projection * vec4(coord, 1, 1);
}
