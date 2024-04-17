#include "vulkanengine_model.hpp"

// std
#include <cassert>
#include <cstring>

namespace vulkanengine
{
	VulkanEngineModel::VulkanEngineModel(VulkanEngineDevice& device, const std::vector<Vertex>& vertices) : vulkanengine_device_(device)
	{
		CreateVertexBuffers(vertices);
	}

	VulkanEngineModel::~VulkanEngineModel()
	{
		vkDestroyBuffer(vulkanengine_device_.Device(), vertex_buffer_, nullptr);
		vkFreeMemory(vulkanengine_device_.Device(), vertex_buffer_memory_, nullptr);
	}

	void VulkanEngineModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertex_count_ = static_cast<uint32_t>(vertices.size());

		assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
		vulkanengine_device_.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertex_buffer_,
			vertex_buffer_memory_);

		void* data;
		vkMapMemory(vulkanengine_device_.Device(), vertex_buffer_memory_, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(vulkanengine_device_.Device(), vertex_buffer_memory_);
	}

	void VulkanEngineModel::Bind(VkCommandBuffer command_buffer)
	{
		VkBuffer buffers[] = { vertex_buffer_ };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
	}

	void VulkanEngineModel::Draw(VkCommandBuffer command_buffer)
	{
		vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> VulkanEngineModel::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(Vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanEngineModel::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);

		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);

		return attribute_descriptions;
	}

} // namespace vulkanengine