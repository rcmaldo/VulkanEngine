#include "vulkanengine_game_object.hpp"

namespace vulkanengine
{
	// mat4 => (translate * rot.y * rot.x * rot.z * scale) transformation
	// Rotation convention is Tait-Bryan angles with axis order Y(1), X(2), Z(3)
	// Matrix formula derived from Wikipedia's Euler Angles article under Rotation Matrix section
	// Notes: For Euler Angles, "intrinsic rotation" is read from left to right, "extrinsic rotation" is read from right to left
	// Notes: glm::mat4 is written by column
	glm::mat4 TransformComponent::Mat4()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x* (c1* c3 + s1 * s2 * s3),
				scale.x* (c2* s3),
				scale.x* (c1* s2* s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{ translation.x, translation.y, translation.z, 1.0f }
		};
	}

	glm::mat3 TransformComponent::NormalMatrix()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		const glm::vec3 inv_scale = 1.0f / scale;

		return glm::mat3{
			{
				inv_scale.x* (c1* c3 + s1 * s2 * s3),
				inv_scale.x* (c2* s3),
				inv_scale.x* (c1* s2* s3 - c3 * s1),
			},
			{
				inv_scale.y * (c3 * s1 * s2 - c1 * s3),
				inv_scale.y * (c2 * c3),
				inv_scale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				inv_scale.z * (c2 * s1),
				inv_scale.z * (-s2),
				inv_scale.z * (c1 * c2),
			},
		};
	}

	VulkanEngineGameObject VulkanEngineGameObject::CreatePointLight(float intensity, float radius, glm::vec3 color)
	{
		VulkanEngineGameObject game_object = CreateGameObject();
		game_object.color_ = color;
		game_object.transform_.scale.x = radius;
		game_object.point_light_ = std::make_unique<PointLightComponent>();
		game_object.point_light_->light_intensity = intensity;
		return game_object;
	}
} // namespace vulkanengine