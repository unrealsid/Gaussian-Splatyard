#pragma once

#include <vulkan/vulkan_core.h>
#include "renderer/Subpass.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "structs/scene/SplatRenderingParams.h"

namespace core::rendering
{
    class ImGuiPass : public Subpass
    {
    public:
        ImGuiPass(EngineContext& engine_context, uint32_t max_frames_in_flight);
        ~ImGuiPass() override;

        void subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container) override;
        void render_target_init(EngineRenderTargets& render_targets) override;
        void frame_pre_recording() override;
        void record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders,
                             GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables) override;
        void cleanup() override;

        bool is_pass_active() const override { return true; }

    private:
        SplatRenderingParams splat_rendering_params{};
        VkDescriptorPool imgui_pool{};

        void init_imgui();
        void map_splat_rendering_params();
    };
}
