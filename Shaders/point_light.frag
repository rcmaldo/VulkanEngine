#version 450

layout(location = 0) in vec2 frag_offset;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
	vec4 ambient_light_color; // w is intensity
	vec3 light_position;
	vec4 light_color; // w is intensity
} ubo;

void main() {
	float distance_from_center = sqrt(dot(frag_offset, frag_offset));
	if (distance_from_center > 1.0)
	{
		discard;
	}
	out_color = vec4(ubo.light_color.xyz, 1.0);
}