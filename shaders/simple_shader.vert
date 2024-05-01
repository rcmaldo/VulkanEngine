#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_pos_world;
layout(location = 2) out vec3 frag_normal_world;

struct PointLight
{
	vec4 position;
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
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
	vec4 position_worldspace = push.model_matrix * vec4(position, 1.0);
	gl_Position = ubo.projection_matrix * ubo.view_matrix * position_worldspace;

	frag_normal_world = normalize(mat3(push.normal_matrix) * normal);
	frag_pos_world = position_worldspace.xyz;
	frag_color = color;
}