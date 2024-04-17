#pragma once

#include "Engine/vulkanengine_device.hpp"
#include "Engine/vulkanengine_game_object.hpp"
#include "Engine/vulkanengine_renderer.hpp"
#include "Engine/vulkanengine_window.hpp"

// std
#include <memory>
#include <vector>

namespace vulkanengine
{
	class FirstApp
	{
	public:
		static constexpr int kWidth = 800;
		static constexpr int kHeight = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();

	private:
		void LoadGameObjects();

		VulkanEngineWindow vulkanengine_window_{ kWidth, kHeight, "Hello Vulkan!" };
		VulkanEngineDevice vulkanengine_device_{ vulkanengine_window_ };
		VulkanEngineRenderer vulkanengine_renderer_{ vulkanengine_window_, vulkanengine_device_ };

		std::vector<VulkanEngineGameObject> game_objects_;
	};
}  // namespace vulkanengine