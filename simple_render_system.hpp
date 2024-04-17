#pragma once

#include "vulkanengine_device.hpp"
#include "vulkanengine_game_object.hpp"
#include "vulkanengine_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace vulkanengine
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VulkanEngineDevice& device, VkRenderPass render_pass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void RenderGameObjects(VkCommandBuffer command_buffer, std::vector<VulkanEngineGameObject>& game_objects);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass render_pass);

		VulkanEngineDevice& vulkanengine_device_;

		std::unique_ptr<VulkanEnginePipeline> vulkanengine_pipeline_;
		VkPipelineLayout pipeline_layout_;
	};
}  // namespace vulkanengine
