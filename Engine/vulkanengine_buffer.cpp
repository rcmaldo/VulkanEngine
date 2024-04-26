/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "vulkanengine_buffer.hpp"

 // std
#include <cassert>
#include <cstring>

namespace vulkanengine
{

    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instance_size The size of an instance
     * @param min_offset_alignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize VulkanEngineBuffer::GetAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment)
    {
        if (min_offset_alignment > 0)
        {
            return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return instance_size;
    }

    VulkanEngineBuffer::VulkanEngineBuffer(
        VulkanEngineDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : vulkanengine_device_{ device },
        instance_size_{ instanceSize },
        instance_count_{ instanceCount },
        usage_flags_{ usageFlags },
        memory_property_flags_{ memoryPropertyFlags }
    {
        alignment_size_ = GetAlignment(instanceSize, minOffsetAlignment);
        buffer_size_ = alignment_size_ * instanceCount;
        device.CreateBuffer(buffer_size_, usageFlags, memoryPropertyFlags, buffer_, memory_);
    }

    VulkanEngineBuffer::~VulkanEngineBuffer()
    {
        Unmap();
        vkDestroyBuffer(vulkanengine_device_.Device(), buffer_, nullptr);
        vkFreeMemory(vulkanengine_device_.Device(), memory_, nullptr);
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult VulkanEngineBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(buffer_ && memory_ && "Called map on buffer before create");
        return vkMapMemory(vulkanengine_device_.Device(), memory_, offset, size, 0, &mapped_);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void VulkanEngineBuffer::Unmap()
    {
        if (mapped_)
        {
            vkUnmapMemory(vulkanengine_device_.Device(), memory_);
            mapped_ = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void VulkanEngineBuffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mapped_ && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mapped_, data, buffer_size_);
        }
        else
        {
            char* mem_offset = (char*)mapped_;
            mem_offset += offset;
            memcpy(mem_offset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult VulkanEngineBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return vkFlushMappedMemoryRanges(vulkanengine_device_.Device(), 1, &mapped_range);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult VulkanEngineBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return vkInvalidateMappedMemoryRanges(vulkanengine_device_.Device(), 1, &mapped_range);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo VulkanEngineBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo{
            buffer_,
            offset,
            size,
        };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    void VulkanEngineBuffer::WriteToIndex(void* data, int index)
    {
        WriteToBuffer(data, instance_size_, index * alignment_size_);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult VulkanEngineBuffer::FlushIndex(int index) { return Flush(alignment_size_, index * alignment_size_); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo VulkanEngineBuffer::DescriptorInfoForIndex(int index)
    {
        return DescriptorInfo(alignment_size_, index * alignment_size_);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult VulkanEngineBuffer::InvalidateIndex(int index)
    {
        return Invalidate(alignment_size_, index * alignment_size_);
    }

}  // namespace vulkanengine