#pragma once
#include "renderer/Subpass.h"

namespace core::rendering
{
    class ScreenspacePass : public Subpass
    {
    public:
        ScreenspacePass(EngineContext& engine_context, uint32_t max_frames_in_flight);

        void subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container) override;
        void render_target_init(EngineRenderTargets& render_targets) override;
        void frame_pre_recording() override;
        void record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders, class
                             GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables) override ;

        bool is_pass_active() const override { return false; }

        void setup_descriptor_layout();
        void push_descriptors(
            VkCommandBuffer cmd,
            VkPipelineLayout pipeline_layout,
            VkDescriptorType descriptor_type,
            uint32_t set_number,
            const VkImageView* image_views,
            uint32_t image_count,
            VkImageLayout image_layout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,
            uint32_t first_binding = 0) const;

        void cleanup() override;

        private:
            VkDescriptorSetLayout descriptor_set_layout{};
    };
}
