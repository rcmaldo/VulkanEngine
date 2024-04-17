#include "vulkanengine_renderer.hpp"

// std
#include <stdexcept>
#include <cassert>
#include <array>

namespace vulkanengine
{
	VulkanEngineRenderer::VulkanEngineRenderer(VulkanEngineWindow& window, VulkanEngineDevice& device)
		: vulkanengine_window_{window}, vulkanengine_device_{device}
	{
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	VulkanEngineRenderer::~VulkanEngineRenderer()
	{
		FreeCommandBuffers();
	}

	void VulkanEngineRenderer::RecreateSwapChain()
	{
		auto extent = vulkanengine_window_.GetExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = vulkanengine_window_.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vulkanengine_device_.Device());

		if (vulkanengine_swap_chain_ == nullptr)
		{
			vulkanengine_swap_chain_ = std::make_unique<VulkanEngineSwapChain>(vulkanengine_device_, extent);
		}
		else
		{
			std::shared_ptr<VulkanEngineSwapChain> old_swap_chain = std::move(vulkanengine_swap_chain_);
			vulkanengine_swap_chain_ = std::make_unique<VulkanEngineSwapChain>(vulkanengine_device_, extent, old_swap_chain);

			if (!old_swap_chain->CompareSwapFormats(*vulkanengine_swap_chain_.get()))
			{
				throw std::runtime_error("Swap chain image or depth format has changed!");
			}
		}

		// CreatePipeline();
	}

	void VulkanEngineRenderer::CreateCommandBuffers()
	{
		command_buffers_.resize(VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = vulkanengine_device_.GetCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

		if (vkAllocateCommandBuffers(vulkanengine_device_.Device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void VulkanEngineRenderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(
			vulkanengine_device_.Device(),
			vulkanengine_device_.GetCommandPool(),
			static_cast<uint32_t>(command_buffers_.size()),
			command_buffers_.data());
		command_buffers_.clear();
	}

	VkCommandBuffer VulkanEngineRenderer::BeginFrame()
	{
		assert(!is_frame_started_ && "Can't call BeginFrame while frame is already in progress");

		auto result = vulkanengine_swap_chain_->AcquireNextImage(&current_image_index_);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}

		is_frame_started_ = true;

		auto command_buffer = GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return command_buffer;
	}

	void VulkanEngineRenderer::EndFrame()
	{
		assert(is_frame_started_ && "Can't call EndFrame while frame is not in progress");
		auto command_buffer = GetCurrentCommandBuffer();
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = vulkanengine_swap_chain_->SubmitCommandBuffers(&command_buffer, &current_image_index_);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanengine_window_.WasWindowResized())
		{
			vulkanengine_window_.ResetWindowResizedFlag();
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image");
		}

		is_frame_started_ = false;
		current_frame_index_ = (current_frame_index_ + 1) % VulkanEngineSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanEngineRenderer::BeginSwapChainRenderPass(VkCommandBuffer command_buffer)
	{
		assert(is_frame_started_ && "Can't call BeginSwapChainRenderPass while frame is not in progress");
		assert(command_buffer && GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = vulkanengine_swap_chain_->GetRenderPass();
		render_pass_info.framebuffer = vulkanengine_swap_chain_->GetFrameBuffer(current_image_index_);

		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = vulkanengine_swap_chain_->GetSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };	// index 0 is the color attachment
		clear_values[1].depthStencil = { 1.0f, 0 };			// index 1 is the depth attachment
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vulkanengine_swap_chain_->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(vulkanengine_swap_chain_->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vulkanengine_swap_chain_->GetSwapChainExtent() };
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void VulkanEngineRenderer::EndSwapChainRenderPass(VkCommandBuffer command_buffer)
	{
		assert(is_frame_started_ && "Can't call EndSwapChainRenderPass while frame is not in progress");
		assert(command_buffer && GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(command_buffer);
	}

}  // namespace vulkanengine
