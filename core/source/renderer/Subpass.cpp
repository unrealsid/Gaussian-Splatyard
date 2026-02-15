#include "renderer/Subpass.h"

#include "structs/EngineContext.h"
#include "structs/Renderable.h"
#include "vulkanapp/utils/ImageUtils.h"
#include "vulkanapp/utils/RenderUtils.h"

namespace core::rendering
{
    Subpass::Subpass(EngineContext& engine_context, uint32_t max_frames_in_flight) :
        engine_context(engine_context),
        max_frames_in_flight(max_frames_in_flight),
        active_command_buffer(nullptr), depth_attachment_info({})
    {
        swapchain_manager = engine_context.swapchain_manager.get();
        device_manager = engine_context.device_manager.get();
        draw_state = std::make_unique<renderer::DrawState>(engine_context);
    }

    void Subpass::init_pass_new_frame(VkCommandBuffer p_command_buffer, uint32_t p_frame)
    {
        current_frame = p_frame;
        active_command_buffer = p_command_buffer;
    }

    void Subpass::begin_command_buffer_recording() const
    {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (engine_context.dispatch_table.beginCommandBuffer(active_command_buffer, &begin_info) != VK_SUCCESS)
        {
            std::cout << "failed to begin recording command buffer\n";
            assert(false);
        }
    }

    void Subpass::setup_depth_attachment(const RenderingAttachmentInfoData& depth_stencil_attachment_info)
    {
        // Depth attachment
        depth_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_attachment_info.pNext       = VK_NULL_HANDLE;
        depth_attachment_info.imageView   = depth_stencil_attachment_info.image_view;
        depth_attachment_info.imageLayout =  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
        depth_attachment_info.loadOp      = depth_stencil_attachment_info.load_op;
        depth_attachment_info.storeOp     = depth_stencil_attachment_info.store_op;
        depth_attachment_info.clearValue  = depth_stencil_attachment_info.surface_clear_value; // depth=1, stencil=0
    }

    void Subpass::begin_rendering(const std::vector<VkRenderingAttachmentInfo>& attachment_infos) const
    {
        // Render area
        VkExtent2D extent = swapchain_manager->get_extent();
        VkRect2D render_area = { {0, 0}, {extent.width, extent.height} };

        // Dynamic rendering info
        VkRenderingInfoKHR render_info = { VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
        render_info.renderArea          = render_area;
        render_info.layerCount          = 1;
        render_info.colorAttachmentCount = attachment_infos.size();
        render_info.pColorAttachments   = attachment_infos.data();
        render_info.pDepthAttachment    = &depth_attachment_info;
        render_info.pStencilAttachment  = &depth_attachment_info;

        engine_context.dispatch_table.cmdBeginRenderingKHR(active_command_buffer, &render_info);
    }

    void Subpass::end_rendering() const
    {
        engine_context.dispatch_table.cmdEndRenderingKHR(active_command_buffer);
    }

    void Subpass::cleanup()
    {

    }
}
