#version 150 core
in vec2 frag_tex_coord;

out vec4 out_color;

uniform sampler2D tex;
uniform vec3 color;

void main() {
    out_color = vec4(1, 1, 1, texture(tex, frag_tex_coord).r)*vec4(color, 1);
}
