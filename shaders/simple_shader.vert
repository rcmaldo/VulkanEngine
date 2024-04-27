#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 frag_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_view_matrix;
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
	vec4 position_worldspace = push.model_matrix * vec4(position, 1.0);
	gl_Position = ubo.projection_view_matrix * position_worldspace;

	vec3 normal_worldspace = normalize(mat3(push.normal_matrix) * normal);

	vec3 direction_to_light = ubo.light_position - position_worldspace.xyz;
	float attenuation = 1.0 / dot(direction_to_light, direction_to_light);
	
	vec3 light_color = ubo.light_color.xyz * ubo.light_color.w * attenuation;
	vec3 ambient_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
	vec3 diffuse_light = light_color * max(dot(normal_worldspace, normalize(direction_to_light)), 0);
	
    frag_color = (ambient_light + diffuse_light) * color;
}