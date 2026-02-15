#include "renderer/DrawState.h"
#include "structs/EngineContext.h"


namespace renderer
{
    DrawState::DrawState(EngineContext& context) : engine_context(context)
    {
        color_blend_equations.resize(2);
    }

    void DrawState::set_and_apply_viewport_scissor(VkCommandBuffer cmd_buffer, VkExtent2D vp_extent,
                                                    VkExtent2D sc_extents, VkOffset2D sc_offset)
    {
        // Store state
        viewport_extent = vp_extent;
        scissor_extents = sc_extents;
        scissor_offset = sc_offset;

        // Apply immediately
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(viewport_extent.width);
        viewport.height = static_cast<float>(viewport_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = scissor_offset;
        scissor.extent = scissor_extents;

        engine_context.dispatch_table.cmdSetViewportWithCountEXT(cmd_buffer, 1, &viewport);
        engine_context.dispatch_table.cmdSetScissorWithCountEXT(cmd_buffer, 1, &scissor);
        engine_context.dispatch_table.cmdSetScissor(cmd_buffer, 0, 1, &scissor);
    }

    void DrawState::set_and_apply_vertex_input(VkCommandBuffer cmd_buffer,
                                                const std::vector<VkVertexInputBindingDescription2EXT>& binding,
                                                const std::vector<VkVertexInputAttributeDescription2EXT>& attributes)
    {
        // Store state
        vertex_input_binding = binding;
        input_attribute_description = attributes;

        // Apply immediately
        engine_context.dispatch_table.cmdSetVertexInputEXT(
            cmd_buffer,
            vertex_input_binding.size(),
            vertex_input_binding.data(),
            input_attribute_description.size(),
            input_attribute_description.data()
        );
    }

    void DrawState::set_and_apply_color_blend(VkCommandBuffer cmd_buffer,
                                               const std::vector<VkColorComponentFlags>& flags,
                                               const std::vector<VkBool32>& enables)
    {
        // Store state
        color_component_flags = flags;
        color_blend_enables = enables;

        // Apply immediately
        engine_context.dispatch_table.cmdSetColorBlendEnableEXT(cmd_buffer, 0, color_blend_enables.size(), color_blend_enables.data());
        engine_context.dispatch_table.cmdSetColorWriteMaskEXT(cmd_buffer, 0, color_component_flags.size(), color_component_flags.data());
    }

    void DrawState::set_blend_equation(uint32_t attachment_id, VkBlendFactor src_color_blend_factor,
                                       VkBlendFactor dst_color_blend_factor, VkBlendOp color_blend_op, VkBlendFactor src_alpha_blend_factor,
                                       VkBlendFactor dst_alpha_blend_factor, VkBlendOp alpha_blend_op)
    {
        assert(attachment_id < 10);

        VkColorBlendEquationEXT color_blend_equation = {};
        color_blend_equation.srcColorBlendFactor = src_color_blend_factor;
        color_blend_equation.dstColorBlendFactor = dst_color_blend_factor;
        color_blend_equation.colorBlendOp = color_blend_op;
        color_blend_equation.srcAlphaBlendFactor = src_alpha_blend_factor;
        color_blend_equation.dstAlphaBlendFactor = dst_alpha_blend_factor;
        color_blend_equation.alphaBlendOp = alpha_blend_op;

        color_blend_equations[attachment_id] = (color_blend_equation);
    }

    void DrawState::apply_blend_equation(VkCommandBuffer cmd_buffer) const
    {
        engine_context.dispatch_table.cmdSetColorBlendEquationEXT(cmd_buffer, 0, color_blend_equations.size(), color_blend_equations.data());
    }

    void DrawState::apply_rasterization_state(VkCommandBuffer cmd_buffer) const
    {
        engine_context.dispatch_table.cmdSetCullModeEXT(cmd_buffer, VK_CULL_MODE_NONE);
        engine_context.dispatch_table.cmdSetFrontFaceEXT(cmd_buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        engine_context.dispatch_table.cmdSetPrimitiveTopologyEXT(cmd_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        engine_context.dispatch_table.cmdSetRasterizerDiscardEnableEXT(cmd_buffer, VK_FALSE);
        engine_context.dispatch_table.cmdSetPolygonModeEXT(cmd_buffer, VK_POLYGON_MODE_FILL);
        engine_context.dispatch_table.cmdSetRasterizationSamplesEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT);
        engine_context.dispatch_table.cmdSetAlphaToCoverageEnableEXT(cmd_buffer, VK_FALSE);
        engine_context.dispatch_table.cmdSetDepthBiasEnableEXT(cmd_buffer, VK_FALSE);
        engine_context.dispatch_table.cmdSetPrimitiveRestartEnableEXT(cmd_buffer, VK_FALSE);

        const VkSampleMask sample_mask = 0xFF;
        engine_context.dispatch_table.cmdSetSampleMaskEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT, &sample_mask);
    }

    void DrawState::apply_depth_stencil_state(VkCommandBuffer cmd_buffer, VkBool32 depth_test_enabled, VkBool32 depth_write_enabled, VkCompareOp depth_compare_op) const
    {
        engine_context.dispatch_table.cmdSetDepthTestEnableEXT(cmd_buffer, depth_test_enabled);
        engine_context.dispatch_table.cmdSetDepthWriteEnableEXT(cmd_buffer, depth_write_enabled);
        engine_context.dispatch_table.cmdSetDepthCompareOpEXT(cmd_buffer, depth_compare_op); //VK_COMPARE_OP_LESS
        engine_context.dispatch_table.cmdSetStencilTestEnableEXT(cmd_buffer, VK_FALSE);
    }

    void DrawState::apply_all_draw_states(VkCommandBuffer cmd_buffer) const
    {
        // Apply viewport and scissor
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(viewport_extent.width);
        viewport.height = static_cast<float>(viewport_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = scissor_offset;
        scissor.extent = scissor_extents;

        engine_context.dispatch_table.cmdSetViewportWithCountEXT(cmd_buffer, 1, &viewport);
        engine_context.dispatch_table.cmdSetScissorWithCountEXT(cmd_buffer, 1, &scissor);
        engine_context.dispatch_table.cmdSetScissor(cmd_buffer, 0, 1, &scissor);

        // Apply rasterization
        apply_rasterization_state(cmd_buffer);

        // Apply depth stencil
        apply_depth_stencil_state(cmd_buffer);

        // Apply color blend
        engine_context.dispatch_table.cmdSetColorBlendEnableEXT(cmd_buffer, 0, color_blend_enables.size(), color_blend_enables.data());
        engine_context.dispatch_table.cmdSetColorWriteMaskEXT(cmd_buffer, 0, color_component_flags.size(), color_component_flags.data());

        // Apply vertex input
        engine_context.dispatch_table.cmdSetVertexInputEXT(
            cmd_buffer,
            vertex_input_binding.size(),
            vertex_input_binding.data(),
            input_attribute_description.size(),
            input_attribute_description.data()
        );
    }
} // material