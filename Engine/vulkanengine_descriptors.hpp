#pragma once

#include "vulkanengine_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vulkanengine
{
    class VulkanEngineDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(VulkanEngineDevice& device) : vulkanengine_device_{ device } {}

            Builder& AddBinding(
                uint32_t binding,
                VkDescriptorType descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t count = 1);
            std::unique_ptr<VulkanEngineDescriptorSetLayout> Build() const;

        private:
            VulkanEngineDevice& vulkanengine_device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        VulkanEngineDescriptorSetLayout(VulkanEngineDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VulkanEngineDescriptorSetLayout();
        VulkanEngineDescriptorSetLayout(const VulkanEngineDescriptorSetLayout&) = delete;
        VulkanEngineDescriptorSetLayout& operator=(const VulkanEngineDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptor_set_layout_; }

    private:
        VulkanEngineDevice& vulkanengine_device_;
        VkDescriptorSetLayout descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

        friend class VulkanEngineDescriptorWriter;
    };


    class VulkanEngineDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(VulkanEngineDevice& device) : vulkanengine_device_{ device } {}

            Builder& AddPoolSize(VkDescriptorType descriptor_type, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            std::unique_ptr<VulkanEngineDescriptorPool> Build() const;

        private:
            VulkanEngineDevice& vulkanengine_device_;
            std::vector<VkDescriptorPoolSize> pool_sizes_{};
            uint32_t max_sets_ = 1000;
            VkDescriptorPoolCreateFlags pool_flags_ = 0;
        };

        VulkanEngineDescriptorPool(
            VulkanEngineDevice& device,
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize>& pool_sizes);
        ~VulkanEngineDescriptorPool();
        VulkanEngineDescriptorPool(const VulkanEngineDescriptorPool&) = delete;
        VulkanEngineDescriptorPool& operator=(const VulkanEngineDescriptorPool&) = delete;

        bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool();

    private:
        VulkanEngineDevice& vulkanengine_device_;
        VkDescriptorPool descriptor_pool_;

        friend class VulkanEngineDescriptorWriter;
    };

    class VulkanEngineDescriptorWriter
    {
    public:
        VulkanEngineDescriptorWriter(VulkanEngineDescriptorSetLayout& set_layout, VulkanEngineDescriptorPool& pool);

        VulkanEngineDescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info);
        VulkanEngineDescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* image_info);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        VulkanEngineDescriptorSetLayout& set_layout_;
        VulkanEngineDescriptorPool& pool_;
        std::vector<VkWriteDescriptorSet> writes_;
    };
} // namespace vulkanengine