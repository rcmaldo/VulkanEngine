#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_position_world;
layout(location = 2) in vec3 frag_normal_world;

layout(location = 0) out vec4 out_color;

struct PointLight
{
	vec4 position;
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 inverse_view_matrix;
	vec4 ambient_light_color; // w is intensity
	PointLight point_lights[10]; // value of vulkanengine_frame_info.hpp->MAX_LIGHTS
	int num_lights;
} ubo;

// due to limitations on some GPUs, we can only store 128 bytes (= 2 4x4 matrices) for push constants
layout(push_constant) uniform Push {
	mat4 model_matrix;
	mat4 normal_matrix;
} push;

void main() {
	vec3 diffuse_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
	vec3 specular_light = vec3(0.0);
	vec3 surface_normal = normalize(frag_normal_world);

	vec3 camera_position_world = ubo.inverse_view_matrix[3].xyz;
	vec3 view_direction = normalize(camera_position_world - frag_position_world);

	for (int i = 0; i < ubo.num_lights; ++i)
	{
		PointLight light = ubo.point_lights[i];
		vec3 direction_to_light = light.position.xyz - frag_position_world;
		float attenuation = 1.0 / dot(direction_to_light, direction_to_light);
		direction_to_light = normalize(direction_to_light);

		float cos_angle_incidence = max(dot(surface_normal, direction_to_light), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuse_light += intensity * cos_angle_incidence;

		// specular term
		vec3 half_angle = normalize(direction_to_light + view_direction);
		float specular_term = dot(surface_normal, half_angle);
		specular_term = clamp(specular_term, 0, 1);
		specular_term = pow(specular_term, 512.0);
		specular_light += light.color.xyz * attenuation * specular_term;
	}

	out_color = vec4(diffuse_light * frag_color + specular_light * frag_color, 1.0);
}