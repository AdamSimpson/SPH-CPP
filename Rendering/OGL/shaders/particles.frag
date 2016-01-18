#version 330

layout(std140) uniform view_matrices
{
    mat4 world_to_camera_matrix;
    mat4 camera_to_clip;
};

layout(std140) uniform light
{
    vec4 world_space_position;
    vec4 camera_space_position;
    vec4 intensity;
    vec4 ambient_intensity;
    float attenuation;
} light_data;

in frag_data
{
    flat vec3 camera_sphere_position;
    flat vec3 sphere_color;
    smooth vec2 mapping;
};

out vec4 output_color;

struct material_t
{
    vec4 diffuse_color;
    vec4 specular_color;
    float specular_shininess;
} mtl;

float calc_attenuation(in vec3 camera_space_position,
	out vec3 light_direction)
{
	vec3 light_difference =  light_data.camera_space_position.xyz - camera_space_position;
	float light_distance_squared = dot(light_difference, light_difference);
	light_direction = light_difference * inversesqrt(light_distance_squared);

	return (1 / ( 1.0 + light_data.attenuation * light_distance_squared));
}

uniform float sphere_radius;

vec4 ComputeLighting(in vec3 camera_space_position,
	in vec3 camera_space_normal, in material_t material)
{
	vec3 light_dir;
	vec4 light_intensity;
	if(light_data.camera_space_position.w == 0.0)
	{
		light_dir = vec3(light_data.camera_space_position);
		light_intensity = light_data.intensity;
	}
	else
	{
		float atten = calc_attenuation(camera_space_position, light_dir);
		light_intensity = atten * light_data.intensity;
	}

	vec3 surface_normal = normalize(camera_space_normal);
	float cos_angle_incidence = dot(surface_normal, light_dir);
	cos_angle_incidence = cos_angle_incidence < 0.0001 ? 0.0 : cos_angle_incidence;

	vec3 view_direction = normalize(-camera_space_position);

	vec3 halfAngle = normalize(light_dir + view_direction);
	float angle_normal_half = acos(dot(halfAngle, surface_normal));
	float exponent = angle_normal_half / material.specular_shininess;
	exponent = -(exponent * exponent);
	float gaussian_term = exp(exponent);

	gaussian_term = cos_angle_incidence != 0.0 ? gaussian_term : 0.0;

	vec4 lighting = material.diffuse_color * light_intensity * cos_angle_incidence;
	lighting += material.specular_color * light_intensity * gaussian_term;

	return lighting;
}

void Impostor(out vec3 camera_position, out vec3 camera_normal)
{
	vec3 camera_plane_position = vec3(mapping * sphere_radius, 0.0) + camera_sphere_position;
	vec3 ray_direction = normalize(camera_plane_position);

	float B = 2.0 * dot(ray_direction, -camera_sphere_position);
	float C = dot(camera_sphere_position, camera_sphere_position) -
		(sphere_radius * sphere_radius);

	float det = (B * B) - (4 * C);
	if(det < 0.0)
		discard;

	float sqrtDet = sqrt(det);
	float posT = (-B + sqrtDet)/2;
	float negT = (-B - sqrtDet)/2;

	float intersectT = min(posT, negT);
	camera_position = ray_direction * intersectT;
	camera_normal = normalize(camera_position - camera_sphere_position);
}

void main()
{
        mtl.diffuse_color = vec4(sphere_color, 0.98);
        mtl.specular_color = vec4(0.8, 0.8, 0.8, 0.98);
        mtl.specular_shininess = 0.1;

	vec3 camera_position;
	vec3 camera_normal;

	Impostor(camera_position, camera_normal);

	//Set the depth based on the new camera_position.
	vec4 clip_position = camera_to_clip * vec4(camera_position, 1.0);
	float ndc_depth = clip_position.z / clip_position.w;
	gl_FragDepth = ((gl_DepthRange.diff * ndc_depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;

	vec4 accum_lighting = mtl.diffuse_color * light_data.ambient_intensity;
	accum_lighting += ComputeLighting(camera_position, camera_normal, mtl);

	output_color = accum_lighting;
  output_color.a = 0.98;
}
