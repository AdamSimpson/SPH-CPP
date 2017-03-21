#version 330

uniform vec4 color;

in vec4 camera_space_frag_pos;
in vec4 camera_space_normal;
in vec2 frag_tex_coord;

out vec4 out_color;

layout(std140) uniform view_matrices
{
    mat4 world_to_camera_matrix;
    mat4 camera_to_clip;
};

layout(std140) uniform light
{
    vec4 world_space_pos;
    vec4 camera_space_pos;
    vec4 intensity;
    vec4 ambient_intensity;
    float attenuation;
} LightData;

float calc_attenuation(in vec3 camera_space_position,
                      in vec3 camera_space_light_pos,
                      out vec3 light_direction)
{
        vec3 light_difference =  camera_space_light_pos - camera_space_position;
        float light_distance_sqrd = dot(light_difference, light_difference);
        light_direction = light_difference * inversesqrt(light_distance_sqrd);

        return (1 / ( 1.0 + 1.3 * light_distance_sqrd));
}

void main() {
    vec3 surface_to_light = vec3(0.0);
    vec3 light_pos = LightData.camera_space_pos.xyz;
    vec3 frag_pos =  camera_space_frag_pos.xyz;
    float atten_intensity = calc_attenuation(frag_pos, light_pos, surface_to_light);

    float cos_angle_incidence = dot(normalize(camera_space_normal.xyz), surface_to_light);
    cos_angle_incidence = clamp(cos_angle_incidence, 0, 1);

    vec4 checker_color = color;

    int num_checks = 30;
    if ((int(floor(num_checks*frag_tex_coord.x) + floor(num_checks*frag_tex_coord.y)) & 1) == 0) {
        checker_color = vec4(0.8, 0.8, 0.8, 1.0);
    }

    out_color = (checker_color * LightData.intensity * atten_intensity *
                 cos_angle_incidence) + (checker_color * LightData.ambient_intensity);
}
