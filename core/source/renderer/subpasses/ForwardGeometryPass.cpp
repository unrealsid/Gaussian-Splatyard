#include "renderer/subpasses/ForwardGeometryPass.h"

#include "3d/ModelUtils.h"
#include "config/Config.inl"
#include "materials/MaterialUtils.h"
#include "structs/EngineContext.h"
#include "structs//geometry/Vertex.h"
#include "structs/scene/PushConstantBlock.h"
#include "enums/inputs/UIAction.h"
#include "vulkanapp/VulkanCleanupQueue.h"
#include "vulkanapp/utils/MemoryUtils.h"
#include "renderer/GPU_BufferContainer.h"
#include "common/Types.h"
#include "vulkanapp/utils/ImageUtils.h"
#include "vulkanapp/utils/RenderUtils.h"

namespace core::rendering
{
    ForwardGeometryPass::ForwardGeometryPass(EngineContext& engine_context, uint32_t max_frames_in_flight) : Subpass(engine_context, max_frames_in_flight){}

    void ForwardGeometryPass::render_target_init(EngineRenderTargets& render_targets)
    {
        //Assign images to the render target
        render_targets.swapchain_images = swapchain_manager->get_images();
        render_targets.swapchain_extent = swapchain_manager->get_extent();
        extents = render_targets.swapchain_extent;
    }

    void ForwardGeometryPass::subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container)
    {
        extents = swapchain_manager->get_extent();

        //Placeholder scene initialization
        // auto gaussian_surfaces = std::vector<glm::vec4>{ {0.0, 0.0, 0.0, 1.0} };
        // auto alphas = std::vector<float>{ 0.0 };
        //
        // buffer_container.allocate_named_buffer("positions", gaussian_surfaces);
        // buffer_container.allocate_named_buffer("scales", gaussian_surfaces);
        // buffer_container.allocate_named_buffer("colors", gaussian_surfaces);
        // buffer_container.allocate_named_buffer("quaternions", gaussian_surfaces);
        // buffer_container.allocate_named_buffer("alpha", alphas);
        // buffer_container.gaussian_count = 1;

        //Register a new event to allocate memory when a new model is loaded
        engine_context.ui_action_manager->register_string_action(UIAction::ALLOCATE_SPLAT_MEMORY,
             [this, &buffer_container](const std::string& code)
             {
                 engine_context.dispatch_table.deviceWaitIdle();

                 std::string str = R"(D:\Data\point_cloud_truck_30k.ply)";
                 auto gaussian_surfaces = entity_3d::ModelUtils::load_gaussian_surfaces(str);
                 //entity_3d::ModelUtils::load_placeholder_gaussian_model();

                 const auto& positions = gaussian_surfaces.get_positions();
                 const auto& scales = gaussian_surfaces.get_scales();
                 const auto& colors = gaussian_surfaces.get_colors_sh();
                 const auto& quaternions = gaussian_surfaces.get_quaternions();
                 const auto& alpha = gaussian_surfaces.get_alphas();

                 buffer_container.allocate_named_buffer("positions", positions);
                 buffer_container.allocate_named_buffer("scales", scales);
                 buffer_container.allocate_named_buffer("colors", colors);
                 buffer_container.allocate_named_buffer("quaternions", quaternions);
                 buffer_container.allocate_named_buffer("alpha", alpha);

                 buffer_container.gaussian_count = gaussian_surfaces.get_count();
             });
    }

    void ForwardGeometryPass::frame_pre_recording(){ }

    void ForwardGeometryPass::record_commands(VkCommandBuffer* command_buffer, uint32_t image_index,
                                              SubpassShaderList& subpass_shaders,
                                              GPU_BufferContainer& buffer_container,
                                              EngineRenderTargets& render_targets,
                                              const std::vector<Renderable>& renderables)
    {
        auto image = render_targets.swapchain_images[image_index];

        //Create the color attachments for this pass -- decides what gets rendered
        auto color_attachments = utils::RenderUtils::create_color_attachments(
        {
            {
                image.image_view,
                {0.0f, 0.0f, 0.0f, 1.0f},
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            }
        });//vector

        setup_depth_attachment(
        {
            render_targets.depth_stencil_image->view,
            {1.0f},
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        });

        //Set layout transition for swapchain image
        utils::ImageUtils::image_layout_transition(*command_buffer, image.image,
                                                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                                                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                0,
                                                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        //Set Layout transition for depth image
        utils::ImageUtils::image_layout_transition(*command_buffer, render_targets.depth_stencil_image->image,
                                                     VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                                     VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                                     0,
                                                     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                      VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });


        begin_rendering(color_attachments);

        // std::vector<VkColorComponentFlags> color_component_flags = {0xF};
        // std::vector color_blend_enables = {VK_FALSE};
        //
        // // Set Draw state params
        // draw_state->set_and_apply_viewport_scissor(*command_buffer, swapchain_manager->get_extent(), swapchain_manager->get_extent(), {0, 0});
        // draw_state->set_and_apply_color_blend(*command_buffer, color_component_flags, color_blend_enables);
        //
        // draw_state->apply_rasterization_state(*command_buffer);
        // draw_state->apply_depth_stencil_state(*command_buffer, VK_TRUE, VK_TRUE);
        //
        // draw_state->set_and_apply_vertex_input(*command_buffer, GaussianSurfaceDescriptor::get_binding_descriptions(), GaussianSurfaceDescriptor::get_attribute_descriptions());
        //
        // subpass_shaders[ShaderObjectType::OpaquePass]->get_shader_object()->bind_material_shader(engine_context.dispatch_table, *command_buffer);
        //
        // for (const auto& renderable : renderables)
        // {
        //     if (renderable.material_index == 0)
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
        //         engine_context.dispatch_table.cmdPushConstants(*command_buffer, subpass_shaders[ShaderObjectType::OpaquePass]->get_pipeline_layout(),
        //                                                                 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        //                                                                 0, sizeof(PushConstantBlock), &push_constant_block);
        //
        //         engine_context.dispatch_table.cmdDraw(*command_buffer, renderable.vertex_count, 1, 0, 0);
        //     }
        // }
    }

    void ForwardGeometryPass::cleanup()
    {
        Subpass::cleanup();

        engine_context.buffer_container->destroy_buffers();
    }
}
