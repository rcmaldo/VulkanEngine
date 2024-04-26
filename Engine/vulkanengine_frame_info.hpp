#pragma once

#include "vulkanengine_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vulkanengine
{
	struct FrameInfo
	{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		VulkanEngineCamera& camera;
	};
} // namespace vulkanengine