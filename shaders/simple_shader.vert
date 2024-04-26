#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 frag_color;

// due to limitations on some GPUs, we can only store 128 bytes (= 2 4x4 matrices) for push constants
layout(push_constant) uniform Push {
	mat4 transform;	// projection * view * model
	mat4 normal_matrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

void main() {
	gl_Position = push.transform * vec4(position, 1.0);

	// This is GPU intensive; calculate normal matrix in CPU instead
    // mat3 normal_matrix = transpose(inverse(mat3(push.model_matrix)));
    // vec3 normal_worldspace = normalize(normal_matrix * normal);

	vec3 normal_worldspace = normalize(mat3(push.normal_matrix) * normal);
	
    float light_intensity = AMBIENT + max(dot(normal_worldspace, DIRECTION_TO_LIGHT), 0);
	
    frag_color = light_intensity * color;
}