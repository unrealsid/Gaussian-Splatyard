#include "vulkanapp/utils/RenderUtils.h"
#include <iostream>
#include "VkBootstrap.h"
#include "vulkanapp/DeviceManager.h"
#include "structs/Vk_Image.h"
#include "structs/EngineContext.h"
#include "vulkanapp/utils/ImageUtils.h"

bool utils::RenderUtils::create_command_pool(const EngineContext& engine_context, VkCommandPool& out_command_pool)
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = engine_context.device_manager->get_device().get_queue_index(vkb::QueueType::graphics).value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    if (engine_context.dispatch_table.createCommandPool(&pool_info, nullptr, &out_command_pool) != VK_SUCCESS)
    {
        std::cout << "failed to create command pool\n";
        return false;
    }

    return true;
}

bool utils::RenderUtils::allocate_command_buffers(const EngineContext& render_context,
                                                  VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    if (render_context.dispatch_table.allocateCommandBuffers(&allocInfo, command_buffers.data()) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

bool utils::RenderUtils::allocate_command_buffer(const EngineContext& render_context, VkCommandPool command_pool, VkCommandBuffer& command_buffer)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (render_context.dispatch_table.allocateCommandBuffers(&allocInfo, &command_buffer) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

VkBool32 utils::RenderUtils::get_supported_depth_stencil_format(VkPhysicalDevice physical_device, VkFormat* depth_stencil_format)
{
    std::vector formatList =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (auto& format : formatList)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depth_stencil_format = format;
            return true;
        }
    }

    return false;
}

std::vector<VkRenderingAttachmentInfoKHR> utils::RenderUtils::create_color_attachments(const std::vector<RenderingAttachmentInfoData>& rendering_attachment_info_data)
{
    std::vector<VkRenderingAttachmentInfoKHR> color_attachments;

    for (const auto& data : rendering_attachment_info_data)
    {
        VkRenderingAttachmentInfoKHR color_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };

        color_attachment_info.pNext        = VK_NULL_HANDLE;
        color_attachment_info.imageView    = data.image_view;
        color_attachment_info.imageLayout  = data.image_layout;
        color_attachment_info.resolveMode  = VK_RESOLVE_MODE_NONE;
        color_attachment_info.loadOp       = data.load_op;
        color_attachment_info.storeOp      = data.store_op;
        color_attachment_info.clearValue   = data.surface_clear_value;

        color_attachments.push_back(color_attachment_info);
    }

    return color_attachments;
}

VkRenderingInfoKHR utils::RenderUtils::rendering_info(VkRect2D render_area, uint32_t color_attachment_count,
                                                      const VkRenderingAttachmentInfoKHR* pColorAttachments, VkRenderingFlagsKHR flags)
{
    VkRenderingInfoKHR rendering_info   = {};
    rendering_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    rendering_info.pNext                = VK_NULL_HANDLE;
    rendering_info.flags                = flags;
    rendering_info.renderArea           = render_area;
    rendering_info.layerCount           = 1;
    rendering_info.viewMask             = 0;
    rendering_info.colorAttachmentCount = color_attachment_count;
    rendering_info.pColorAttachments    = pColorAttachments;
    rendering_info.pDepthAttachment     = VK_NULL_HANDLE;
    rendering_info.pStencilAttachment   = VK_NULL_HANDLE;
    return rendering_info;
}
