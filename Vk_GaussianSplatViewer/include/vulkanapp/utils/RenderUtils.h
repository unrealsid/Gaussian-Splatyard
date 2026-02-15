#pragma once
#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "structs/RenderingAttachmentInfoData.h"
#include "structs/Vk_Image.h"

struct EngineContext;
struct DepthStencilImage;

namespace utils
{
    class RenderUtils
    {
    public:
        static bool create_command_pool(const EngineContext& engine_context, VkCommandPool& out_command_pool);

        static bool allocate_command_buffers(const EngineContext& render_context, VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers);

        static bool allocate_command_buffer(const EngineContext& render_context, VkCommandPool command_pool, VkCommandBuffer& command_buffer);

        static VkBool32 get_supported_depth_stencil_format(VkPhysicalDevice physical_device, VkFormat* depth_stencil_format);

        static std::vector<VkRenderingAttachmentInfoKHR> create_color_attachments(const std::vector<RenderingAttachmentInfoData>& rendering_attachment_info_data);

        static VkRenderingInfoKHR rendering_info(VkRect2D render_area = {},
                                      uint32_t color_attachment_count = 0,
                                      const VkRenderingAttachmentInfoKHR *pColorAttachments = VK_NULL_HANDLE,
                                      VkRenderingFlagsKHR flags = 0);
    };
}
