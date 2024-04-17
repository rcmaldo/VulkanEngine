#pragma once

#include "vulkanengine_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace vulkanengine
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		// mat4 => (translate * rot.y * rot.x * rot.z * scale) transformation
		// Rotation convention is Tait-Bryan angles with axis order Y(1), X(2), Z(3)
		// Matrix formula derived from Wikipedia's Euler Angles article under Rotation Matrix section
		// Notes: For Euler Angles, "intrinsic rotation" is read from left to right, "extrinsic rotation" is read from right to left
		// Notes: glm::mat4 is written by column
		glm::mat4 mat4()
		{
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
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
	};

	class VulkanEngineGameObject
	{
	public:
		using id_t = unsigned int;

		static VulkanEngineGameObject CreateGameObject()
		{
			static id_t current_id = 0;
			return VulkanEngineGameObject{ current_id++ };
		}

		VulkanEngineGameObject(const VulkanEngineGameObject&) = delete;
		VulkanEngineGameObject& operator=(const VulkanEngineGameObject&) = delete;
		VulkanEngineGameObject(VulkanEngineGameObject&&) = default;
		VulkanEngineGameObject& operator=(VulkanEngineGameObject&&) = default;

		id_t GetId() const
		{
			return id_;
		}

		std::shared_ptr<VulkanEngineModel> model_{};
		glm::vec3 color_{};
		TransformComponent transform_{};

	private:
		VulkanEngineGameObject(id_t object_id) : id_{ object_id } {}

		id_t id_;
	};
} // vulkanengine