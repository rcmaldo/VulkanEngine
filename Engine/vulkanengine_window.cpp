#include "vulkanengine_window.hpp"

// std
#include <stdexcept>
namespace vulkanengine
{

	VulkanEngineWindow::VulkanEngineWindow(int width, int height, std::string window_name)
		: width_{ width }, height_{ height }, window_name_{ window_name }
	{
		InitWindow();
	}

	VulkanEngineWindow::~VulkanEngineWindow()
	{
		glfwDestroyWindow(window_);
		glfwTerminate();  // Disable GLFW
	}

	void VulkanEngineWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}

	void VulkanEngineWindow::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vulkanengine_window = reinterpret_cast<VulkanEngineWindow*>(glfwGetWindowUserPointer(window));
		vulkanengine_window->frame_buffer_resized_ = true;
		vulkanengine_window->width_ = width;
		vulkanengine_window->height_ = height;
	}

	void VulkanEngineWindow::InitWindow()
	{
		glfwInit();  // Enable GLFW: an API for platform agnostic window creation
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, FrameBufferResizeCallback);
	}
}  // namespace vulkanengine
