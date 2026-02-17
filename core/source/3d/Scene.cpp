
#include "3d/Scene.h"

#include <numeric>

#include "3d/ModelUtils.h"
#include "structs/EngineContext.h"

namespace entity_3d
{
    void Scene::populate_scene()
    {
        //Load the quad buffer we'll need for displaying the splat
        core::rendering::GPU_BufferContainer* gpu_buffer_container = engine_context.buffer_container.get();
        auto quad_vertices = ModelUtils::load_quad();
        gpu_buffer_container->allocate_named_buffer("quad_vertices_buffer", quad_vertices);

        std::string str = R"(C:\Users\Sid\Documents\Python\Data\train_point_cloud_30k.ply)";
        engine_context.splat_loader = std::make_unique<splat_loader::GaussianSplatPlyLoader>(ModelUtils::load_gaussian_surfaces(str));
        //entity_3d::ModelUtils::load_placeholder_gaussian_model();

        const auto& positions = engine_context.splat_loader ->get_positions();
        const auto& scales = engine_context.splat_loader->get_scales();
        const auto& colors = engine_context.splat_loader->get_colors_sh();
        const auto& quaternions = engine_context.splat_loader->get_quaternions();
        const auto& alpha = engine_context.splat_loader->get_alphas();

        const auto& indices = engine_context.splat_loader->sort_gaussians(*(engine_context.renderer->get_camera()));

        gpu_buffer_container->allocate_named_buffer("splat_positions_buffer", positions);
        gpu_buffer_container->allocate_named_buffer("splat_scales_buffer", scales);
        gpu_buffer_container->allocate_named_buffer("splat_colors_buffer", colors);
        gpu_buffer_container->allocate_named_buffer("splat_quaternions_buffer", quaternions);
        gpu_buffer_container->allocate_named_buffer("splat_alpha_buffer", alpha);

        gpu_buffer_container->allocate_named_buffer("splat_indices_buffer", indices, BufferAllocationType::MappedAllocation);
        gpu_buffer_container->map_splat_indices(indices);

        gpu_buffer_container->gaussian_count = engine_context.splat_loader->get_count();

        //Visible Indices buffer
        const auto count = static_cast<uint32_t>(positions.size());
        std::vector<uint32_t> visible_indices(count);
        std::iota(visible_indices.begin(), visible_indices.end(), 0);

        //The array of indices visible gaussians
        gpu_buffer_container->allocate_named_buffer("visible_splat_indices_buffer", visible_indices);

        //Visible gaussians count for compute pass
        uint32_t visible_count = 0;
        gpu_buffer_container->allocate_named_buffer_simple<uint32_t>("visible_splat_count_buffer", BufferAllocationType::MappedAllocation);
        gpu_buffer_container->map_data("visible_splat_count_buffer", &visible_count, sizeof(uint32_t));

        //Used as the alternative ping-pong buffer
        std::vector<glm::uvec2> visible_splats_idx_buffer_alt(gpu_buffer_container->gaussian_count);
        gpu_buffer_container->allocate_named_buffer("visible_splat_indices_buffer_alt", visible_splats_idx_buffer_alt);

        constexpr uint32_t NUM_BLOCKS_PER_WORKGROUP = 32;
        constexpr uint32_t WORKGROUP_SIZE = 256;
        uint32_t num_workgroups = (gpu_buffer_container->gaussian_count + (NUM_BLOCKS_PER_WORKGROUP * WORKGROUP_SIZE) - 1) / (NUM_BLOCKS_PER_WORKGROUP * WORKGROUP_SIZE);

        std::vector<uint32_t> histograms(num_workgroups * 256, 0);
        gpu_buffer_container->allocate_named_buffer("histogram_data_buffer", histograms);
    }
}
