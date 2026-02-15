#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

struct GPU_Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info{};
    VkDeviceAddress buffer_address{};
};
