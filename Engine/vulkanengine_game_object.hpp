#pragma once

#include "vulkanengine_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace vulkanengine
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		glm::mat4 Mat4();
		glm::mat3 NormalMatrix();
	};

	class VulkanEngineGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VulkanEngineGameObject>;

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
} // namespace vulkanengine