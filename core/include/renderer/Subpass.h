#pragma once

#include "materials/Material.h"
#include "platform/WindowManager.h"
#include "common/Types.h"
#include "renderer/DrawState.h"
#include "structs/Renderable.h"
#include "structs/RenderingAttachmentInfoData.h"

struct EngineRenderTargets;
struct PushConstantBlock;
struct EngineContext;
struct WindowCreateParams;

namespace vulkanapp
{
    class SwapchainManager;
    class DeviceManager;
}

namespace core::rendering
{
    class GPU_BufferContainer;

    class Subpass
    {
    public:
        explicit Subpass(EngineContext& engine_context, uint32_t max_frames_in_flight = 2);
        virtual ~Subpass() = default;

        //Initializes the subpass render targets
        virtual void render_target_init(EngineRenderTargets& render_targets) = 0;

        //Initializes the subpass and the associated material for the subpass if necessary
        virtual void subpass_init(SubpassShaderList& subpass_shaders,
                                  GPU_BufferContainer& buffer_container) = 0;

        //Called before a frame is recorded
        virtual void frame_pre_recording() = 0;

        //Record commands into the buffer. Place draw commands here
        virtual void record_commands(VkCommandBuffer* command_buffer,
                                     uint32_t image_index,
                                     SubpassShaderList& subpass_shaders,
                                     GPU_BufferContainer& buffer_container,
                                     EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables) = 0;

        virtual bool is_pass_active() const = 0;

        //Cleanup pass
        virtual void cleanup();

        //Initialize parameters for a new frame. Runs at the start of every new frame
        void init_pass_new_frame(VkCommandBuffer p_command_buffer, uint32_t p_frame);

        //Start recording buffer commands
        void begin_command_buffer_recording() const;

        void setup_depth_attachment(const RenderingAttachmentInfoData& depth_stencil_attachment_info);
        void begin_rendering(const std::vector<VkRenderingAttachmentInfo>& attachment_infos) const;
        void end_rendering() const;

    protected:
        EngineContext& engine_context;

        vulkanapp::SwapchainManager* swapchain_manager;
        vulkanapp::DeviceManager* device_manager;

        uint32_t max_frames_in_flight;

        //Which command buffer are we currently using?
        VkCommandBuffer active_command_buffer;
        uint32_t current_frame = 0;

        //Stores depth attachment used during command recording
        VkRenderingAttachmentInfoKHR depth_attachment_info;

        std::unique_ptr<renderer::DrawState> draw_state;

        VkExtent2D extents{};
    };
}
