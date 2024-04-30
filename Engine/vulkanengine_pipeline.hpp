#pragma once

#include "vulkanengine_device.hpp"

// std
#include <string>
#include <vector>

namespace vulkanengine
{

	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> binding_descriptions{};
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanEnginePipeline
	{
	public:
		VulkanEnginePipeline() = default;
		VulkanEnginePipeline(VulkanEngineDevice& device,
			const std::string& vert_filepath,
			const std::string& frag_filepath,
			const PipelineConfigInfo& config_info);

		~VulkanEnginePipeline();

		VulkanEnginePipeline(const VulkanEnginePipeline&) = delete;
		VulkanEnginePipeline& operator=(const VulkanEnginePipeline&) = delete;

		void Bind(VkCommandBuffer command_buffer);

		static void DefaultPipelineConfigInfo(PipelineConfigInfo& config_info);

	private:
		static std::vector<char> ReadFile(const std::string& filepath);

		void CreateGraphicsPipeline(const std::string& vert_filepath,
			const std::string& frag_filepath,
			const PipelineConfigInfo& config_info);

		void CreateShaderModule(const std::vector<char>& code,
			VkShaderModule* shader_module);

		VulkanEngineDevice& vulkanengine_device_;
		VkPipeline graphics_pipeline_;
		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;
	};
}  // namespace vulkanengine