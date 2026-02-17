#include "renderer/subpasses/TranslucencyPass.h"

#include "config/Config.inl"
#include "materials/MaterialUtils.h"
#include "structs/scene/PushConstantBlock.h"
#include "common/Types.h"
#include "platform/DirectoryPath.h"
#include "structs/EngineContext.h"
#include "structs/EngineRenderTargets.h"
#include "structs/geometry/GaussianSurface.h"
#include "vulkanapp/utils/ImageUtils.h"
#include "vulkanapp/utils/RenderUtils.h"


namespace core::rendering
{
    void TranslucencyPass::subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container)
    {
        auto gaussian_transparent_vs = platform::DirectoryPath::get().get_shader_path("gaussian_surface", "gaussian_transparent.vert");
        auto gaussian_transparent_fs = platform::DirectoryPath::get().get_shader_path("gaussian_surface", "gaussian_transparent.frag");

        material::MaterialUtils material_utils(engine_context);
        subpass_shaders[ShaderObjectType::TranslucentPass] = material_utils.create_material<PushConstantBlock>("translucent_pass",
                                                                                                gaussian_transparent_vs,
                                                                                                gaussian_transparent_fs, nullptr, 0);
    }

    void TranslucencyPass::render_target_init(EngineRenderTargets& render_targets)
    {
        auto extents = render_targets.swapchain_extent;
        constexpr VmaAllocationCreateInfo alloc_info{};
        render_targets.accumulation_image = std::make_unique<Vk_Image>(utils::ImageUtils::create_image(engine_context, extents.width, extents.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,  alloc_info));
        render_targets.revealage_image = std::make_unique<Vk_Image>(utils::ImageUtils::create_image(engine_context, extents.width, extents.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,  alloc_info));
    }

    void TranslucencyPass::frame_pre_recording(){}

    void TranslucencyPass::record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders, class
                                           GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables)
    {
         std::vector<VkColorComponentFlags> color_component_flags =
         {
             VK_COLOR_COMPONENT_R_BIT |
             VK_COLOR_COMPONENT_G_BIT |
             VK_COLOR_COMPONENT_B_BIT |
             VK_COLOR_COMPONENT_A_BIT,  // outColor
         };

         std::vector color_blend_enables = {VK_TRUE};

         draw_state->set_and_apply_viewport_scissor(*command_buffer, swapchain_manager->get_extent(), swapchain_manager->get_extent(), {0, 0});
         draw_state->set_and_apply_color_blend(*command_buffer, color_component_flags, color_blend_enables);

         //Set blend equation for attachment 0
         draw_state->set_blend_equation(
             0,
             VK_BLEND_FACTOR_SRC_ALPHA,
             VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
             VK_BLEND_OP_ADD,

             VK_BLEND_FACTOR_SRC_ALPHA,
             VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
             VK_BLEND_OP_ADD
         );

         draw_state->apply_blend_equation(*command_buffer);

         draw_state->apply_rasterization_state(*command_buffer);
         draw_state->apply_depth_stencil_state(*command_buffer, VK_FALSE, VK_FALSE);

         draw_state->set_and_apply_vertex_input(*command_buffer, GaussianSurfaceDescriptor::get_binding_descriptions(), GaussianSurfaceDescriptor::get_attribute_descriptions());

         subpass_shaders[ShaderObjectType::TranslucentPass]->get_shader_object()->bind_material_shader(engine_context.dispatch_table, *command_buffer);

        //Vertices
        // for (const auto& renderable : renderables)
        // {
        //     if (renderable.material_index == 1)
        //     {
        //         //Vertices
        //         VkBuffer vertex_buffers[] = { renderable.vertex_buffer.buffer, renderable.vertex_colors_buffer.buffer };
        //         VkDeviceSize offsets[] = {0, 0};
        //         engine_context.dispatch_table.cmdBindVertexBuffers(*command_buffer, 0, 2, vertex_buffers, offsets);
        //
        //         //Push Constants
        //         push_constant_block =
        //         {
        //             .camera_data_address = buffer_container.camera_data_buffer.buffer_address,
        //             .splat_positions_address = buffer_container.get_model_matrix_buffer_offset_address(renderable.object_index)
        //         };
        //
        //         engine_context.dispatch_table.cmdPushConstants(*command_buffer, subpass_shaders[ShaderObjectType::TranslucentPass]->get_pipeline_layout(),
        //                                                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        //                                                               0, sizeof(PushConstantBlock), &push_constant_block);
        //
        //         engine_context.dispatch_table.cmdDraw(*command_buffer, renderable.vertex_count, 1, 0, 0);
        //     }
        // }

        auto quad_vertices_buffer = buffer_container.get_buffer("quad_vertices_buffer");

        auto splat_positions_buffer = buffer_container.get_buffer("splat_positions_buffer");
        auto splat_scales_buffer = buffer_container.get_buffer("splat_scales_buffer");
        auto splat_colors_buffer = buffer_container.get_buffer("splat_colors_buffer");
        auto splat_quaternions_buffer = buffer_container.get_buffer("splat_quaternions_buffer");
        auto splat_alpha_buffer = buffer_container.get_buffer("splat_alpha_buffer");
        auto splat_indices_buffer = buffer_container.get_buffer("splat_indices_buffer");

        VkBuffer vertex_buffers[] = { quad_vertices_buffer->buffer };
        VkDeviceSize offsets[] = {0};
        engine_context.dispatch_table.cmdBindVertexBuffers(*command_buffer, 0, 1, vertex_buffers, offsets);

        //Push Constants
        push_constant_block =
        {
            .camera_data_address = buffer_container.camera_data_buffer.buffer_address,
            .splat_indices_address = splat_indices_buffer->buffer_address,
            .splat_positions_address = splat_positions_buffer->buffer_address,
            .splat_scales_address = splat_scales_buffer->buffer_address,
            .splat_colors_address = splat_colors_buffer->buffer_address,
            .splat_quats_address = splat_quaternions_buffer->buffer_address,
            .splat_alphas_address = splat_alpha_buffer->buffer_address,
            .splat_render_params_address = buffer_container.splat_render_params_buffer.buffer_address
        };

        engine_context.dispatch_table.cmdPushConstants(*command_buffer, subpass_shaders[ShaderObjectType::TranslucentPass]->get_pipeline_layout(),
                                                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                                              0, sizeof(PushConstantBlock), &push_constant_block);

        engine_context.dispatch_table.cmdDraw(*command_buffer, 6, buffer_container.gaussian_count, 0, 0);

        end_rendering();
    }

    void TranslucencyPass::cleanup(){ }
}
