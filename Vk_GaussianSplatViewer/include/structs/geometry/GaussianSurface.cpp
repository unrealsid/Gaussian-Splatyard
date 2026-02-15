#include "GaussianSurface.h"
#include <glm/glm.hpp>

std::vector<VkVertexInputBindingDescription2EXT> GaussianSurfaceDescriptor::get_binding_descriptions()
{
    std::vector<VkVertexInputBindingDescription2EXT> bindings{};
    bindings.resize(1);

    // Binding 0: positions (vec2)
    bindings[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
    bindings[0].pNext = nullptr;
    bindings[0].binding = 0;
    bindings[0].stride = sizeof(glm::vec2);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[0].divisor = 1;

    return bindings;
}

std::vector<VkVertexInputAttributeDescription2EXT> GaussianSurfaceDescriptor::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription2EXT> attributes{};
    attributes.resize(1);

    // Location 0: position (vec4 from binding 0)
    attributes[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
    attributes[0].pNext = nullptr;
    attributes[0].location = 0;
    attributes[0].binding = 0;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = 0;

    return attributes;
}
