#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <cassert>
#include <array>

namespace vulkanengine
{

	struct SimplePushConstantData
	{
		glm::mat2 transform{1.f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	SimpleRenderSystem::SimpleRenderSystem(VulkanEngineDevice& device, VkRenderPass render_pass)
		: vulkanengine_device_{device}
	{
		CreatePipelineLayout();
		CreatePipeline(render_pass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(vulkanengine_device_.Device(), pipeline_layout_, nullptr);
	}

	void SimpleRenderSystem::CreatePipelineLayout()
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(vulkanengine_device_.Device(),
			&pipeline_layout_info, nullptr,
			&pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::CreatePipeline(VkRenderPass render_pass)
	{
		assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config{};
		VulkanEnginePipeline::DefaultPipelineConfigInfo(pipeline_config);
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout_;

		vulkanengine_pipeline_ = std::make_unique<VulkanEnginePipeline>(
			vulkanengine_device_, "shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv", pipeline_config);
	}

	void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer command_buffer, std::vector<VulkanEngineGameObject>& game_objects)
	{
		vulkanengine_pipeline_->Bind(command_buffer);

		for (auto& obj : game_objects)
		{
			obj.transform_2d_.rotation = glm::mod(obj.transform_2d_.rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = obj.transform_2d_.translation;
			push.color = obj.color_;
			push.transform = obj.transform_2d_.mat2();


			vkCmdPushConstants(
				command_buffer,
				pipeline_layout_,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model_->Bind(command_buffer);
			obj.model_->Draw(command_buffer);
		}

	}

}  // namespace vulkanengine
