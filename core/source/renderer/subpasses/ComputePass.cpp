#include "renderer/subpasses/ComputePass.h"
#include "structs/EngineContext.h"
#include "structs/scene/PushConstantBlock.h"
#include "materials/MaterialUtils.h"
#include "renderer/GPU_BufferContainer.h"
#include "common/Types.h"
#include "config/Config.inl"
#include "renderer/Renderer.h"

#include "platform/DirectoryPath.h"

namespace core::rendering
{
    void ComputePass::subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container)
    {
        material::MaterialUtils material_utils(engine_context);

        //Culling Pass
        auto culling_shader_path = platform::DirectoryPath::get().get_shader_path("compute", "gaussian_culling.comp");
        subpass_shaders[ShaderObjectType::CullingComputePass] = material_utils.create_compute_material<PushConstantsComputeCulling>("compute_pass", culling_shader_path, nullptr, 0);

        //Histogram Pass
        auto histogram_shader_path = platform::DirectoryPath::get().get_shader_path("compute", "multi_radixsort_histograms.comp");
        subpass_shaders[ShaderObjectType::HistogramComputePass] = material_utils.create_compute_material<PushConstantsComputeHistograms>("compute_pass", histogram_shader_path, nullptr, 0);

        //Radix sort
        auto radixsort_shader_path = platform::DirectoryPath::get().get_shader_path("compute", "multi_radixsort.comp");
        subpass_shaders[ShaderObjectType::SortComputePass] = material_utils.create_compute_material<PushConstantsRadixSort>("compute_pass", radixsort_shader_path, nullptr, 0);
    }

    void ComputePass::render_target_init(EngineRenderTargets& render_targets)
    {
    }

    void ComputePass::frame_pre_recording()
    {
    }

    void ComputePass::record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders,
                                      GPU_BufferContainer& buffer_container,
                                      EngineRenderTargets& render_targets,
                                      const std::vector<Renderable>& renderables)
    {
        uint32_t element_count = buffer_container.gaussian_count;
        if (element_count == 0)
        {
            std::cout << "No elements to render!" << std::endl;
            return;
        }

        auto& dispatch_table = engine_context.dispatch_table;

        auto visible_splat_indices_buffer = buffer_container.get_buffer("visible_splat_indices_buffer");
        auto visible_splat_indices_buffer_alt = buffer_container.get_buffer("visible_splat_indices_buffer_alt");
        auto visible_splat_count_buffer = buffer_container.get_buffer("visible_splat_count_buffer");
        auto splat_indices_buffer = buffer_container.get_buffer("splat_indices_buffer");
        auto pos_buffer = buffer_container.get_buffer("splat_positions_buffer");
        auto histogram_buffer = buffer_container.get_buffer("histogram_data_buffer");

        if (!visible_splat_indices_buffer || !visible_splat_indices_buffer_alt || !visible_splat_count_buffer || !splat_indices_buffer || !pos_buffer || !histogram_buffer)
        {
            return;
        }

        constexpr auto WORKGROUP_SIZE = 512;
        constexpr auto RADIX_SORT_BINS  = 256;
        constexpr auto NUM_BLOCKS_PER_WORKGROUP = WORKGROUP_SIZE / RADIX_SORT_BINS;

        // 1. Culling Pass
        {
            int visible_count = 0;
            buffer_container.map_data("visible_splat_count_buffer", &visible_count, sizeof(int));

            auto& compute_material = subpass_shaders[ShaderObjectType::CullingComputePass];
            push_constants_compute_culling.camera_data_address = buffer_container.camera_data_buffer.buffer_address;
            push_constants_compute_culling.splat_indices_address = splat_indices_buffer->buffer_address;
            push_constants_compute_culling.splat_positions_address = pos_buffer->buffer_address;
            push_constants_compute_culling.reduced_splat_indices_address = visible_splat_indices_buffer->buffer_address;
            push_constants_compute_culling.visible_count_address = visible_splat_count_buffer->buffer_address;
            push_constants_compute_culling.g_num_elements = element_count;

            uint32_t num_groups = (element_count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

            compute_material->get_shader_object()->bind_material_shader(dispatch_table, *command_buffer);
            dispatch_table.cmdPushConstants(*command_buffer, compute_material->get_pipeline_layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantsComputeCulling), &push_constants_compute_culling);
            dispatch_table.cmdDispatch(*command_buffer, num_groups, 1, 1);
        }

        // Memory barrier: Culling -> Histogram
        VkMemoryBarrier2 culling_to_hist_barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        culling_to_hist_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        culling_to_hist_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        culling_to_hist_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        culling_to_hist_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo culling_to_hist_dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        culling_to_hist_dep.memoryBarrierCount = 1;
        culling_to_hist_dep.pMemoryBarriers = &culling_to_hist_barrier;
        dispatch_table.cmdPipelineBarrier2(*command_buffer, &culling_to_hist_dep);

        //TODO: Remove this fence afterwards. Will need to implement an indirect compute pass
        auto compute_culling_fence = engine_context.renderer->get_render_pass()->get_compute_fences();
        dispatch_table.waitForFences(1, &compute_culling_fence[0], VK_TRUE, UINT64_MAX);

        int visible_splats = 0;
        memcpy(&visible_splats, visible_splat_count_buffer->allocation_info.pMappedData, sizeof(int));
        uint32_t num_groups = (visible_splats + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

        // 2. Radix Sort Passes
        VkDeviceAddress ping_address = visible_splat_indices_buffer->buffer_address;
        VkDeviceAddress pong_address = visible_splat_indices_buffer_alt->buffer_address;

        for (uint32_t shift = 0; shift < 32; shift += 8)
        {
            // Histogram
            {
                auto& hist_material = subpass_shaders[ShaderObjectType::HistogramComputePass];
                push_constants_compute_histograms.g_num_elements = visible_splats;
                push_constants_compute_histograms.g_shift = shift;
                push_constants_compute_histograms.g_num_workgroups = num_groups;
                push_constants_compute_histograms.g_num_blocks_per_workgroup = NUM_BLOCKS_PER_WORKGROUP;
                push_constants_compute_histograms.reduced_splat_indices_in_address = ping_address;
                push_constants_compute_histograms.histogram_data_address = histogram_buffer->buffer_address;

                hist_material->get_shader_object()->bind_material_shader(dispatch_table, *command_buffer);

                dispatch_table.cmdPushConstants(*command_buffer, hist_material->get_pipeline_layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantsComputeHistograms), &push_constants_compute_histograms);
                dispatch_table.cmdDispatch(*command_buffer, num_groups, 1, 1);
            }

            // Memory barrier: Histogram -> Sort
            VkMemoryBarrier2 hist_to_sort_barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
            hist_to_sort_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            hist_to_sort_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
            hist_to_sort_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            hist_to_sort_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

            VkDependencyInfo hist_to_sort_dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            hist_to_sort_dep.memoryBarrierCount = 1;
            hist_to_sort_dep.pMemoryBarriers = &hist_to_sort_barrier;
            dispatch_table.cmdPipelineBarrier2(*command_buffer, &hist_to_sort_dep);
            //
            //     // Sort
            //     {
            //         auto& sort_material = subpass_shaders[ShaderObjectType::SortComputePass];
            //         push_constants_radix_sort.g_num_elements = element_count;
            //         push_constants_radix_sort.g_shift = shift;
            //         push_constants_radix_sort.g_num_workgroups = globalInvocationSize;
            //         push_constants_radix_sort.g_num_blocks_per_workgroup = NUM_BLOCKS_PER_WORKGROUP;
            //         push_constants_radix_sort.splat_indices_in_address = ping_address;
            //         push_constants_radix_sort.splat_indices_out_address = pong_address;
            //         push_constants_radix_sort.histogram_data_address = histogram_buffer->buffer_address;
            //
            //         sort_material->get_shader_object()->bind_material_shader(dispatch_table, *command_buffer);
            //         dispatch_table.cmdPushConstants(*command_buffer, sort_material->get_pipeline_layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantsRadixSort), &push_constants_radix_sort);
            //         dispatch_table.cmdDispatch(*command_buffer, globalInvocationSize, 1, 1);
            //     }
            //
            //     // Swap ping-pong
            std::swap(ping_address, pong_address);
        }

            // Memory barrier: Sort -> Next Histogram (or Graphics)
            VkMemoryBarrier2 sort_to_next_barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
            sort_to_next_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            sort_to_next_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
            sort_to_next_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
            sort_to_next_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

            VkDependencyInfo sort_to_next_dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            sort_to_next_dep.memoryBarrierCount = 1;
            sort_to_next_dep.pMemoryBarriers = &sort_to_next_barrier;
            dispatch_table.cmdPipelineBarrier2(*command_buffer, &sort_to_next_dep);
        }

    void ComputePass::cleanup()
    {

    }
} // rendering
