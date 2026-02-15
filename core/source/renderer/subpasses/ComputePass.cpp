#include "renderer/subpasses/ComputePass.h"
#include "structs/EngineContext.h"
#include "structs/scene/PushConstantBlock.h"
#include "materials/MaterialUtils.h"
#include "renderer/GPU_BufferContainer.h"
#include "common/Types.h"
#include "config/Config.inl"
#include "renderer/Renderer.h"
#include <iostream>

#include "platform/DirectoryPath.h"

namespace core::rendering
{
    void ComputePass::subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container)
    {
        auto path = platform::DirectoryPath::get().get_shader_path("compute", "dummy.comp");

        material::MaterialUtils material_utils(engine_context);
        subpass_shaders[ShaderObjectType::ComputePass] = material_utils.create_compute_material("compute_pass", path, nullptr, 0);
    }

    void ComputePass::render_target_init(EngineRenderTargets& render_targets)
    {
    }

    void ComputePass::frame_pre_recording()
    {
    }

    void ComputePass::record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, PushConstantBlock& push_constant_block, SubpassShaderList& subpass_shaders,
                                     GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables)
    {
        auto& material = subpass_shaders[ShaderObjectType::ComputePass];
        auto& dispatch_table = engine_context.dispatch_table;

        // Populate push constants with splat buffer addresses
        push_constant_block.camera_data_address = buffer_container.camera_data_buffer.buffer_address;

        auto pos_buffer = buffer_container.get_buffer("positions");
        if (pos_buffer) push_constant_block.splat_positions_address = pos_buffer->buffer_address;

        auto scale_buffer = buffer_container.get_buffer("scales");
        if (scale_buffer) push_constant_block.splat_scales_address = scale_buffer->buffer_address;

        auto color_buffer = buffer_container.get_buffer("colors");
        if (color_buffer) push_constant_block.splat_colors_address = color_buffer->buffer_address;

        auto quat_buffer = buffer_container.get_buffer("quaternions");
        if (quat_buffer) push_constant_block.splat_quats_address = quat_buffer->buffer_address;

        auto alpha_buffer = buffer_container.get_buffer("alpha");
        if (alpha_buffer) push_constant_block.splat_alphas_address = alpha_buffer->buffer_address;

        push_constant_block.splat_render_params_address = buffer_container.splat_render_params_buffer.buffer_address;

        // Bind compute shader
        material->get_shader_object()->bind_material_shader(dispatch_table, *command_buffer);

        // Push constants
        dispatch_table.cmdPushConstants(*command_buffer, material->get_pipeline_layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantBlock), &push_constant_block);

        // Dispatch
        // Example: dispatch based on buffer size
        uint32_t element_count = buffer_container.gaussian_count;
        if (element_count > 0)
        {
            dispatch_table.cmdDispatch(*command_buffer, (element_count + 255) / 256, 1, 1);
        }

        // Memory barrier to ensure compute writes are visible to graphics
        // Also ensure that the write to compute_output is finished before it's read
        VkMemoryBarrier2 memory_barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        memory_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        memory_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT;

        VkDependencyInfo dependency_info = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependency_info.memoryBarrierCount = 1;
        dependency_info.pMemoryBarriers = &memory_barrier;

        dispatch_table.cmdPipelineBarrier2(*command_buffer, &dependency_info);
    }

    void ComputePass::cleanup()
    {
    }
} // rendering
