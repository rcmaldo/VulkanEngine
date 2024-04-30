#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_position_world;
layout(location = 2) in vec3 frag_normal_world;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
	vec4 ambient_light_color; // w is intensity
	vec3 light_position;
	vec4 light_color; // w is intensity
} ubo;

// due to limitations on some GPUs, we can only store 128 bytes (= 2 4x4 matrices) for push constants
layout(push_constant) uniform Push {
	mat4 model_matrix;
	mat4 normal_matrix;
} push;

void main() {
	vec3 direction_to_light = ubo.light_position - frag_position_world;
	float attenuation = 1.0 / dot(direction_to_light, direction_to_light);
	
	vec3 light_color = ubo.light_color.xyz * ubo.light_color.w * attenuation;
	vec3 ambient_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
	vec3 diffuse_light = light_color * max(dot(normalize(frag_normal_world), normalize(direction_to_light)), 0);

	out_color = vec4((ambient_light + diffuse_light) * frag_color, 1.0);
}