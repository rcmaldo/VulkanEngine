#pragma once

#include "vulkanengine_window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace vulkanengine
{

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t presentFamily;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
	};

	class VulkanEngineDevice
	{
	public:
#ifdef NDEBUG
		const bool enable_validation_layers_ = false;
#else
		const bool enable_validation_layers_ = true;
#endif

		VulkanEngineDevice(VulkanEngineWindow& window);
		~VulkanEngineDevice();

		// Not copyable or movable
		VulkanEngineDevice(const VulkanEngineDevice&) = delete;
		VulkanEngineDevice& operator=(const VulkanEngineDevice&) = delete;
		VulkanEngineDevice(VulkanEngineDevice&&) = delete;
		VulkanEngineDevice& operator=(VulkanEngineDevice&&) = delete;

		VkCommandPool GetCommandPool() { return command_pool_; }
		VkDevice Device() { return device_; }
		VkSurfaceKHR Surface() { return surface_; }
		VkQueue GraphicsQueue() { return graphics_queue_; }
		VkQueue PresentQueue() { return present_queue_; }

		SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(physical_device_); }
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(physical_device_); }
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer Helper Functions
		void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(
			VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void CreateImageWithInfo(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);

		VkPhysicalDeviceProperties properties_;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		// helper functions
		bool IsDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void HasGlfwRequiredInstanceExtensions();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		VkInstance instance_;
		VkDebugUtilsMessengerEXT debug_messenger_;
		VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
		VulkanEngineWindow& window_;
		VkCommandPool command_pool_;

		VkDevice device_;
		VkSurfaceKHR surface_;
		VkQueue graphics_queue_;
		VkQueue present_queue_;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};

}  // namespace lve