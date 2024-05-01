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

	struct PointLightComponent
	{
		float light_intensity = 1.0f;
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

		static VulkanEngineGameObject CreatePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		VulkanEngineGameObject(const VulkanEngineGameObject&) = delete;
		VulkanEngineGameObject& operator=(const VulkanEngineGameObject&) = delete;
		VulkanEngineGameObject(VulkanEngineGameObject&&) = default;
		VulkanEngineGameObject& operator=(VulkanEngineGameObject&&) = default;

		id_t GetId() const
		{
			return id_;
		}

		glm::vec3 color_{};
		TransformComponent transform_{};

		std::shared_ptr<VulkanEngineModel> model_{};
		std::unique_ptr<PointLightComponent> point_light_ = nullptr;

	private:
		VulkanEngineGameObject(id_t object_id) : id_{ object_id } {}

		id_t id_;
	};
} // namespace vulkanengine