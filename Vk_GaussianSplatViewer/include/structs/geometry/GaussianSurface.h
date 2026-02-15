#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct GaussianSurfaceDescriptor
{
    static std::vector<VkVertexInputBindingDescription2EXT> get_binding_descriptions();

    static std::vector<VkVertexInputAttributeDescription2EXT> get_attribute_descriptions();
};

