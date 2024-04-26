#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 0) out vec4 out_color;

// due to limitations on some GPUs, we can only store 128 bytes (= 2 4x4 matrices) for push constants
layout(push_constant) uniform Push {
	mat4 transform;	// projection * view * model
	mat4 normal_matrix;
} push;

void main() {
	out_color = vec4(frag_color, 1.0);
}