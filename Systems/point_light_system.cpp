#include "point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

namespace vulkanengine
{
	struct PointLightPushConstants
	{
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(VulkanEngineDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
		: vulkanengine_device_{ device }
	{
		CreatePipelineLayout(global_set_layout);
		CreatePipeline(render_pass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(vulkanengine_device_.Device(), pipeline_layout_, nullptr);
	}

	void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout global_set_layout)
	{
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;
		if (vkCreatePipelineLayout(vulkanengine_device_.Device(),
			&pipeline_layout_info, nullptr,
			&pipeline_layout_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightSystem::CreatePipeline(VkRenderPass render_pass)
	{
		assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config{};
		VulkanEnginePipeline::DefaultPipelineConfigInfo(pipeline_config);
		VulkanEnginePipeline::EnableAlphaBlending(pipeline_config);
		pipeline_config.attribute_descriptions.clear();
		pipeline_config.binding_descriptions.clear();
		pipeline_config.render_pass = render_pass;
		pipeline_config.pipeline_layout = pipeline_layout_;

		vulkanengine_pipeline_ = std::make_unique<VulkanEnginePipeline>(
			vulkanengine_device_,
			"Shaders/point_light.vert.spv",
			"Shaders/point_light.frag.spv",
			pipeline_config);
	}

	void PointLightSystem::Update(FrameInfo& frame_info, GlobalUbo& ubo)
	{
		auto rotate_light = glm::rotate(
			glm::mat4(1.f),
			frame_info.frame_time,
			{ 0.f, -1.f, 0.f });

		int light_index = 0;
		for (auto& kv : frame_info.game_objects)
		{
			auto& object = kv.second;
			if (object.point_light_ == nullptr)
			{
				continue;
			}

			assert(light_index < MAX_LIGHTS && "Point lights exceed maximum specified");

			// update light position
			object.transform_.translation = glm::vec3(rotate_light * glm::vec4(object.transform_.translation, 1.0f));

			// copy light to ubo
			ubo.point_lights[light_index].position = glm::vec4(object.transform_.translation, 1.0f);
			ubo.point_lights[light_index].color = glm::vec4(object.color_, object.point_light_->light_intensity);
			light_index += 1;
		}
		ubo.num_lights = light_index;
	}

	void PointLightSystem::Render(FrameInfo& frame_info)
	{
		std::map<float, VulkanEngineGameObject::id_t> sorted_objects;
		for (auto& kv : frame_info.game_objects)
		{
			auto& obj = kv.second;
			if (obj.point_light_ == nullptr) continue;

			auto offset = frame_info.camera.GetPosition() - obj.transform_.translation;
			float distance_squared = glm::dot(offset, offset);
			sorted_objects[distance_squared] = obj.GetId();
		}

		vulkanengine_pipeline_->Bind(frame_info.command_buffer);

		vkCmdBindDescriptorSets(
			frame_info.command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout_,
			0,
			1,
			&frame_info.global_descriptor_set,
			0,
			nullptr);

		for (auto it = sorted_objects.rbegin(); it != sorted_objects.rend(); ++it)
		{
			auto& object = frame_info.game_objects.at(it->second);

			PointLightPushConstants push{};
			push.position = glm::vec4(object.transform_.translation, 1.f);
			push.color = glm::vec4(object.color_, object.point_light_->light_intensity);
			push.radius = object.transform_.scale.x;

			vkCmdPushConstants(
				frame_info.command_buffer,
				pipeline_layout_,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);

			vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
		}
	}

}  // namespace vulkanengine