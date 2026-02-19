#pragma once

#include "renderer/Subpass.h"
#include "structs/scene/PushConstantBlock.h"

namespace core::rendering
{
    class ComputePass : public Subpass
    {
    public:
        ComputePass(EngineContext& engine_context, uint32_t max_frames_in_flight)
            : Subpass(engine_context, max_frames_in_flight){}

        void subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container) override;
        void render_target_init(EngineRenderTargets& render_targets) override;
        void frame_pre_recording() override;
        void record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders,
                             GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables) override;
        void cleanup() override;

        bool is_pass_active() const override { return true; }

    private:
        PushConstantsComputeCulling push_constants_compute_culling{};
        PushConstantsComputeHistograms push_constants_compute_histograms{};
        PushConstantsRadixSort push_constants_radix_sort{};

        VkDescriptorSetLayout culling_descriptor_layout = VK_NULL_HANDLE;
    };
} // rendering
