#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout(location = 0) out vec2 frag_offset;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
	vec4 ambient_light_color; // w is intensity
	vec3 light_position;
	vec4 light_color; // w is intensity
} ubo;

const float LIGHT_RADIUS = 0.1;

void main()
{
	frag_offset = OFFSETS[gl_VertexIndex];

	vec4 position_viewspace = ubo.view_matrix * vec4(ubo.light_position.xyz, 1.0) + vec4(LIGHT_RADIUS * OFFSETS[gl_VertexIndex], ubo.light_position.z, 0.0);

	gl_Position = ubo.projection_matrix * position_viewspace;
}