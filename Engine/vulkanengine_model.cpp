#include "vulkanengine_model.hpp"

#include "vulkanengine_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std
{
	template <>
	struct hash<vulkanengine::VulkanEngineModel::Vertex>
	{
		size_t operator()(const vulkanengine::VulkanEngineModel::Vertex& vertex) const
		{
			size_t seed = 0;
			vulkanengine::HashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
} // namespace std

namespace vulkanengine
{
	VulkanEngineModel::VulkanEngineModel(VulkanEngineDevice& device, const VulkanEngineModel::Builder& builder) : vulkanengine_device_(device)
	{
		CreateVertexBuffers(builder.vertices);
		CreateIndexBuffers(builder.indices);
	}

	VulkanEngineModel::~VulkanEngineModel()
	{
		vkDestroyBuffer(vulkanengine_device_.Device(), vertex_buffer_, nullptr);
		vkFreeMemory(vulkanengine_device_.Device(), vertex_buffer_memory_, nullptr);

		if (has_index_buffer_)
		{
			vkDestroyBuffer(vulkanengine_device_.Device(), index_buffer_, nullptr);
			vkFreeMemory(vulkanengine_device_.Device(), index_buffer_memory_, nullptr);
		}
	}

	std::unique_ptr<VulkanEngineModel> VulkanEngineModel::CreateModelFromFile(VulkanEngineDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.LoadModel(filepath);

		return std::make_unique<VulkanEngineModel>(device, builder);
	}

	void VulkanEngineModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertex_count_ = static_cast<uint32_t>(vertices.size());

		assert(vertex_count_ >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		vulkanengine_device_.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);

		void* data;
		vkMapMemory(vulkanengine_device_.Device(), staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(vulkanengine_device_.Device(), staging_buffer_memory);

		vulkanengine_device_.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertex_buffer_,
			vertex_buffer_memory_);

		vulkanengine_device_.CopyBuffer(staging_buffer, vertex_buffer_, buffer_size);

		vkDestroyBuffer(vulkanengine_device_.Device(), staging_buffer, nullptr);
		vkFreeMemory(vulkanengine_device_.Device(), staging_buffer_memory, nullptr);
	}

	void VulkanEngineModel::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		index_count_ = static_cast<uint32_t>(indices.size());
		has_index_buffer_ = index_count_ > 0;

		if (!has_index_buffer_)
		{
			return;
		}

		VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		vulkanengine_device_.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);

		void* data;
		vkMapMemory(vulkanengine_device_.Device(), staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(vulkanengine_device_.Device(), staging_buffer_memory);

		vulkanengine_device_.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			index_buffer_,
			index_buffer_memory_);

		vulkanengine_device_.CopyBuffer(staging_buffer, index_buffer_, buffer_size);

		vkDestroyBuffer(vulkanengine_device_.Device(), staging_buffer, nullptr);
		vkFreeMemory(vulkanengine_device_.Device(), staging_buffer_memory, nullptr);
	}

	void VulkanEngineModel::Bind(VkCommandBuffer command_buffer)
	{
		VkBuffer buffers[] = { vertex_buffer_ };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

		if (has_index_buffer_)
		{
			vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void VulkanEngineModel::Draw(VkCommandBuffer command_buffer)
	{
		if (has_index_buffer_)
		{
			vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
		}
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

	void VulkanEngineModel::Builder::LoadModel(const std::string& filepath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> unique_vertices{};
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					auto color_index = 3 * index.vertex_index + 2;
					if (color_index < attrib.colors.size())
					{
						vertex.color = {
							attrib.colors[color_index - 2],
							attrib.colors[color_index - 1],
							attrib.colors[color_index - 0]
						};
					}
					else
					{
						vertex.color = { 1.f, 1.f, 1.f };
					}
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (unique_vertices.count(vertex) == 0)
				{
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(unique_vertices[vertex]);
			}
		}
	}

} // namespace vulkanengine