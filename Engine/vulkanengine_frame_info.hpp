#pragma once

#include "vulkanengine_camera.hpp"
#include "vulkanengine_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vulkanengine
{
#define MAX_LIGHTS 10

	struct PointLight
	{
		glm::vec4 position{};
		glm::vec4 color{}; // w is intensity
	};

	// Global Uniform Buffer Object
	struct GlobalUbo
	{
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f}; // w is light intensity
		PointLight point_lights[MAX_LIGHTS];
		int num_lights;
	};

	struct FrameInfo
	{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		VulkanEngineCamera& camera;
		VkDescriptorSet global_descriptor_set;
		VulkanEngineGameObject::Map& game_objects;
	};
} // namespace vulkanengine