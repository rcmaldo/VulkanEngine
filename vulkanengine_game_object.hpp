#pragma once

#include "vulkanengine_model.hpp"

// std
#include <memory>

namespace vulkanengine
{
	struct Transform2dComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{1.f, 1.f};
		float rotation;

		glm::mat2 mat2()
		{
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotation_mat{{c, s}, { -s, c }};

			// DO NOT FORGET: mat2 is	[ a  b ]
			// {{ a, c }, { b, d }}		[ c  d ]
			glm::mat2 scale_mat{{scale.x, .0f}, { .0f, scale.y }};

			// DO NOT FORGET: transformation order is from right to left
			// due to transform_matrix * position_vector"
			return rotation_mat * scale_mat;
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

		std::shared_ptr< VulkanEngineModel> model_{};
		glm::vec3 color_{};
		Transform2dComponent transform_2d_;

	private:
		VulkanEngineGameObject(id_t object_id) : id_{ object_id } {}

		id_t id_;
	};
} // vulkanengine