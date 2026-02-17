#pragma once

#include "camera/FirstPersonCamera.h"
#include "renderer/Subpass.h"
#include "structs/geometry/GaussianSurface.h"

/*
 * Used to draw opaque geometry
 */

namespace core::rendering
{
    class ForwardGeometryPass : public Subpass
    {
    public:
        ForwardGeometryPass(EngineContext& engine_context, uint32_t max_frames_in_flight);

        void render_target_init(EngineRenderTargets& render_targets) override;
        void subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container) override;
        void frame_pre_recording() override;
        void record_commands(VkCommandBuffer* command_buffer, uint32_t image_index,
                             SubpassShaderList& subpass_shaders,
                             GPU_BufferContainer& buffer_container,
                             EngineRenderTargets& render_targets,
                             const std::vector<Renderable>& renderables) override;

        [[nodiscard]] bool is_pass_active() const override { return true; }

        void cleanup() override;

    private:
        VkExtent2D extents{};
    };
}
