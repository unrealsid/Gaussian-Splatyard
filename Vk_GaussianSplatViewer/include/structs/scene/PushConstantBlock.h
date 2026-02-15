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