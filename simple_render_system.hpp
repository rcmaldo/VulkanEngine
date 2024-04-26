#pragma once

#include "Engine/vulkanengine_camera.hpp"
#include "Engine/vulkanengine_device.hpp"
#include "Engine/vulkanengine_frame_info.hpp"
#include "Engine/vulkanengine_game_object.hpp"
#include "Engine/vulkanengine_pipeline.hpp"

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

		void RenderGameObjects(FrameInfo& frame_info, std::vector<VulkanEngineGameObject>& game_objects);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass render_pass);

		VulkanEngineDevice& vulkanengine_device_;

		std::unique_ptr<VulkanEnginePipeline> vulkanengine_pipeline_;
		VkPipelineLayout pipeline_layout_;
	};
}  // namespace vulkanengine
