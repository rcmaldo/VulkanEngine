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

layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

void main()
{
	frag_offset = OFFSETS[gl_VertexIndex];

	vec4 position_viewspace = ubo.view_matrix * vec4(push.position.xyz, 1.0) + vec4(push.radius * frag_offset, 0.0, 0.0);

	gl_Position = ubo.projection_matrix * position_viewspace;
}