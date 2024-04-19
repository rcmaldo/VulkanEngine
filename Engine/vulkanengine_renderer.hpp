#pragma once

#include "vulkanengine_device.hpp"
#include "vulkanengine_swap_chain.hpp"
#include "vulkanengine_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace vulkanengine
{
	class VulkanEngineRenderer
	{
	public:
		VulkanEngineRenderer(VulkanEngineWindow& window, VulkanEngineDevice& device);
		~VulkanEngineRenderer();

		VulkanEngineRenderer(const VulkanEngineRenderer&) = delete;
		VulkanEngineRenderer& operator=(const VulkanEngineRenderer&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return vulkanengine_swap_chain_->GetRenderPass(); }
		float GetAspectRatio() const { return vulkanengine_swap_chain_->ExtentAspectRatio(); }
		bool IsFrameInProgress() const { return is_frame_started_; }

		VkCommandBuffer GetCurrentCommandBuffer() const {
			assert(is_frame_started_ && "Cannot get command buffer when frame is not in progress");
			return command_buffers_[current_frame_index_];
		}

		int GetFrameIndex() const
		{
			assert(is_frame_started_ && "Cannot get frame index when frame is not in progress");
			return current_frame_index_;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapChainRenderPass(VkCommandBuffer command_buffer);
		void EndSwapChainRenderPass(VkCommandBuffer command_buffer);

	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

		VulkanEngineWindow& vulkanengine_window_;
		VulkanEngineDevice& vulkanengine_device_;
		std::unique_ptr<VulkanEngineSwapChain> vulkanengine_swap_chain_;
		std::vector<VkCommandBuffer> command_buffers_;

		uint32_t current_image_index_;
		int current_frame_index_{ 0 };
		bool is_frame_started_{ false };
	};
}  // namespace vulkanengine
