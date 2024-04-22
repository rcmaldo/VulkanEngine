#pragma once

#include "vulkanengine_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace vulkanengine
{
	class VulkanEngineModel
	{
	public:

		// Make sure to change attribute descriptions when making changes to this struct!
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return
					position	== other.position	&&
					color		== other.color		&&
					normal		== other.normal		&&
					uv			== other.uv;
			}
		};

		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void LoadModel(const std::string& filepath);
		};

		VulkanEngineModel(VulkanEngineDevice& device, const VulkanEngineModel::Builder& builder);
		~VulkanEngineModel();

		VulkanEngineModel(const VulkanEngineModel&) = delete;
		VulkanEngineModel& operator=(const VulkanEngineModel&) = delete;

		static std::unique_ptr<VulkanEngineModel> CreateModelFromFile(VulkanEngineDevice& device, const std::string& filepath);

		void Bind(VkCommandBuffer command_buffer);
		void Draw(VkCommandBuffer command_buffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);

		VulkanEngineDevice& vulkanengine_device_;

		VkBuffer vertex_buffer_;
		VkDeviceMemory vertex_buffer_memory_;
		uint32_t vertex_count_;

		bool has_index_buffer_ = false;
		VkBuffer index_buffer_;
		VkDeviceMemory index_buffer_memory_;
		uint32_t index_count_;
	};
} // namespace vulkanengine