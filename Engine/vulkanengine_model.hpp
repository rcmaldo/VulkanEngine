#pragma once

#include "vulkanengine_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace vulkanengine
{
	class VulkanEngineModel
	{
	public:

		struct Vertex
		{
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		VulkanEngineModel(VulkanEngineDevice& device, const std::vector<Vertex>& vertices);
		~VulkanEngineModel();

		VulkanEngineModel(const VulkanEngineModel&) = delete;
		VulkanEngineModel& operator=(const VulkanEngineModel&) = delete;

		void Bind(VkCommandBuffer command_buffer);
		void Draw(VkCommandBuffer command_buffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);

		VulkanEngineDevice& vulkanengine_device_;
		VkBuffer vertex_buffer_;
		VkDeviceMemory vertex_buffer_memory_;
		uint32_t vertex_count_;
	};
} // namespace vulkanengine