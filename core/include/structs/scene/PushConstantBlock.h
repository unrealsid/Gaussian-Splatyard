#pragma once

#include <vulkan/vulkan_core.h>

struct PushConstantBlock
{
    VkDeviceAddress camera_data_address;
    VkDeviceAddress splat_indices_address;
    VkDeviceAddress splat_positions_address;
    VkDeviceAddress splat_scales_address;
    VkDeviceAddress splat_colors_address;
    VkDeviceAddress splat_quats_address;
    VkDeviceAddress splat_alphas_address;
    VkDeviceAddress splat_render_params_address;
};

struct PushConstantsComputeCulling
{
    VkDeviceAddress camera_data_address;
    VkDeviceAddress splat_indices_address;
    VkDeviceAddress splat_positions_address;

    //Stores splats currently in view
    VkDeviceAddress reduced_splat_indices_address;
    VkDeviceAddress visible_count_address;
};

struct PushConstantsComputeHistograms
{
    uint32_t g_num_elements; // == NUM_ELEMENTS
    uint32_t g_shift;
    uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
    uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP

    VkDeviceAddress reduced_splat_indices_in_address;
    VkDeviceAddress histogram_data_address;
};

struct PushConstantsRadixSort
{
    uint32_t g_num_elements; // == NUM_ELEMENTS
    uint32_t g_shift; // (*)
    uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
    uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP

    VkDeviceAddress splat_indices_in_address;
    VkDeviceAddress splat_indices_out_address;
    VkDeviceAddress histogram_data_address;
};