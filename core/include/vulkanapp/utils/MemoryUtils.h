#pragma once
#include <vector>

#include "VkBootstrapDispatch.h"
#include "structs/Vk_Image.h"

namespace vulkanapp
{
    class DeviceManager;
}

struct GPU_Buffer;
struct EngineContext;
struct MaterialParams;
struct VMAAllocator;

namespace utils
{
    class MemoryUtils
    {
    public: 
        static void create_vma_allocator(vulkanapp::DeviceManager& device_manager);

        static void create_buffer(const vkb::DispatchTable& dispatch_table, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                                    VmaMemoryUsage memory_usage, VmaAllocationCreateFlags vmaAllocationFlags, GPU_Buffer& out_buffer);

        static VkResult map_persistent_data(VmaAllocator vmaAllocator, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, const void* data, VkDeviceSize bufferSize, size_t offset_in_buffer = 0);

        static VkDeviceAddress get_buffer_device_address(const vkb::DispatchTable& disp, VkBuffer buffer);

        static void copy_buffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer, VkBuffer dst_buffer, VkDeviceSize size);

        template <class V>
        static void create_vertex_buffer_with_staging(EngineContext& engine_context, const std::vector<V>& vertices,
                                                      VkCommandPool command_pool, GPU_Buffer& out_vertex_buffer);
        template <class V>
        static void create_index_buffer_with_staging(EngineContext& engine_context,
                                                     const std::vector<uint32_t>& indices,
                                                     VkCommandPool command_pool, GPU_Buffer& out_index_buffer);

        template<typename V>
        static void create_vertex_and_index_buffers(EngineContext& engine_context,
                                                   const std::vector<V>& vertices,
                                                   const std::vector<uint32_t>& indices,
                                                   VkCommandPool command_pool,
                                                   GPU_Buffer& out_vertex_buffer, GPU_Buffer& out_index_buffer);

        template<typename T>
        static void map_ubo(const EngineContext& engine_context, GPU_Buffer buffer, T ubo_data);

        //Creates a device-addressable buffer (Can be addressed via vulkan BDA)
        static void allocate_buffer_with_mapped_access(const vkb::DispatchTable& dispatch_table, VmaAllocator allocator, VkDeviceSize size, GPU_Buffer& buffer);

        //Creates a buffer that is persistently mapped
        static void allocate_buffer_with_random_access(const vkb::DispatchTable& dispatch_table, VmaAllocator allocator, VkDeviceSize size, GPU_Buffer& buffer);

        static void destroy_buffer(VmaAllocator allocator, GPU_Buffer& buffer);
    };
}
