#include "vulkanengine_descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace vulkanengine
{
    // *************** Descriptor Set Layout Builder *********************

    VulkanEngineDescriptorSetLayout::Builder& VulkanEngineDescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        VkDescriptorType descriptor_type,
        VkShaderStageFlags stage_flags,
        uint32_t count)
    {
        assert(bindings_.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = binding;
        layout_binding.descriptorType = descriptor_type;
        layout_binding.descriptorCount = count;
        layout_binding.stageFlags = stage_flags;
        bindings_[binding] = layout_binding;
        return *this;
    }

    std::unique_ptr<VulkanEngineDescriptorSetLayout> VulkanEngineDescriptorSetLayout::Builder::Build() const
    {
        return std::make_unique<VulkanEngineDescriptorSetLayout>(vulkanengine_device_, bindings_);
    }

    // *************** Descriptor Set Layout *********************

    VulkanEngineDescriptorSetLayout::VulkanEngineDescriptorSetLayout(
        VulkanEngineDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : vulkanengine_device_{ device }, descriptor_set_layout_{}, bindings_{ bindings }
    {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto& kv : bindings)
        {
            set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(
            vulkanengine_device_.Device(),
            &descriptor_set_layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VulkanEngineDescriptorSetLayout::~VulkanEngineDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(vulkanengine_device_.Device(), descriptor_set_layout_, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    // Adds [count]x descriptors of descriptor_type in the pool to be used later
    VulkanEngineDescriptorPool::Builder& VulkanEngineDescriptorPool::Builder::AddPoolSize(
        VkDescriptorType descriptor_type, uint32_t count)
    {
        pool_sizes_.push_back({ descriptor_type, count });
        return *this;
    }

    VulkanEngineDescriptorPool::Builder& VulkanEngineDescriptorPool::Builder::SetPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        pool_flags_ = flags;
        return *this;
    }

    // Sets the max amount of descriptor SETS we can create from this pool
    VulkanEngineDescriptorPool::Builder& VulkanEngineDescriptorPool::Builder::SetMaxSets(uint32_t count)
    {
        max_sets_ = count;
        return *this;
    }

    std::unique_ptr<VulkanEngineDescriptorPool> VulkanEngineDescriptorPool::Builder::Build() const
    {
        return std::make_unique<VulkanEngineDescriptorPool>(vulkanengine_device_, max_sets_, pool_flags_, pool_sizes_);
    }

    // *************** Descriptor Pool *********************

    VulkanEngineDescriptorPool::VulkanEngineDescriptorPool(
        VulkanEngineDevice& device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize>& pool_sizes)
        : vulkanengine_device_{ device }
    {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.maxSets = max_sets;
        descriptor_pool_info.flags = pool_flags;

        if (vkCreateDescriptorPool(device.Device(), &descriptor_pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    VulkanEngineDescriptorPool::~VulkanEngineDescriptorPool()
    {
        vkDestroyDescriptorPool(vulkanengine_device_.Device(), descriptor_pool_, nullptr);
    }

    bool VulkanEngineDescriptorPool::AllocateDescriptorSet(
        const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool_;
        alloc_info.pSetLayouts = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(vulkanengine_device_.Device(), &alloc_info, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void VulkanEngineDescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(
            vulkanengine_device_.Device(),
            descriptor_pool_,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void VulkanEngineDescriptorPool::ResetPool()
    {
        vkResetDescriptorPool(vulkanengine_device_.Device(), descriptor_pool_, 0);
    }

    // *************** Descriptor Writer *********************

    VulkanEngineDescriptorWriter::VulkanEngineDescriptorWriter(VulkanEngineDescriptorSetLayout& set_layout, VulkanEngineDescriptorPool& pool)
        : set_layout_{ set_layout }, pool_{ pool }
    {}

    VulkanEngineDescriptorWriter& VulkanEngineDescriptorWriter::WriteBuffer(
        uint32_t binding, VkDescriptorBufferInfo* buffer_info)
    {
        assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        auto& binding_description = set_layout_.bindings_[binding];

        assert(
            binding_description.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = buffer_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    VulkanEngineDescriptorWriter& VulkanEngineDescriptorWriter::WriteImage(
        uint32_t binding, VkDescriptorImageInfo* image_info)
    {
        assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = set_layout_.bindings_[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = image_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    bool VulkanEngineDescriptorWriter::Build(VkDescriptorSet& set)
    {
        bool success = pool_.AllocateDescriptorSet(set_layout_.GetDescriptorSetLayout(), set);
        if (!success)
        {
            return false;
        }
        Overwrite(set);
        return true;
    }

    void VulkanEngineDescriptorWriter::Overwrite(VkDescriptorSet& set)
    {
        for (auto& write : writes_)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool_.vulkanengine_device_.Device(), writes_.size(), writes_.data(), 0, nullptr);
    }
} // namespace vulkanengine