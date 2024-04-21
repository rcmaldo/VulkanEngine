#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vulkanengine
{

	class VulkanEngineWindow
	{
	public:
		VulkanEngineWindow(int width, int height, std::string window_name);
		~VulkanEngineWindow();

		VulkanEngineWindow(const VulkanEngineWindow&) = delete;
		VulkanEngineWindow& operator=(const VulkanEngineWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window_); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(width_), static_cast<uint32_t>(height_) }; }
		bool WasWindowResized() { return frame_buffer_resized_; }
		void ResetWindowResizedFlag() { frame_buffer_resized_ = false; }
		GLFWwindow* GetGLFWwindow() const { return window_; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
		void InitWindow();

		int width_;
		int height_;
		bool frame_buffer_resized_ = false;

		std::string window_name_;
		GLFWwindow* window_;
	};

}  // namespace vulkanengine