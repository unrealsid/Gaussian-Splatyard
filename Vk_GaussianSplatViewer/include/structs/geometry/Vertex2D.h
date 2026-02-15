#pragma once

#include <array>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

struct Vertex2D
{
    glm::vec2 position;
    glm::vec3 color;


    bool operator==(const Vertex2D& other) const
    {
        return position == other.position && color == other.color;
    }

    static VkVertexInputBindingDescription2EXT get_binding_description()
    {
        VkVertexInputBindingDescription2EXT bindingDescription{};
        bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescription.pNext = nullptr;
        bindingDescription.binding = 0; 
        bindingDescription.stride = sizeof(Vertex2D);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.divisor = 1;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription2EXT, 2> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription2EXT, 2> attribute_descriptions{};

        // Position attribute
        attribute_descriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[0].pNext = nullptr;
        attribute_descriptions[0].location = 0; // Corresponds to layout(location = 0) in shader
        attribute_descriptions[0].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // glm::vec2 is 2 floats
        attribute_descriptions[0].offset = offsetof(Vertex2D, position); // Offset within the Vertex struct

        // color attribute
        attribute_descriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[1].pNext = nullptr;
        attribute_descriptions[1].location = 1; // Corresponds to layout(location = 1) in shader
        attribute_descriptions[1].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // glm::vec3 is 3 floats
        attribute_descriptions[1].offset = offsetof(Vertex2D, color); // Offset within the Vertex2D struct

        return attribute_descriptions;
    }
};

// Custom hash function for Vertex, needed for std::unordered_map
namespace std
{
    template<> struct hash<Vertex2D>
    {
        size_t operator()(const Vertex2D& vertex) const noexcept
        {
            // Simple hash combining function for GLM vectors
            auto hashVec3 = [](const glm::vec3& v)
            {
                return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
            };
            auto hashVec2 = [](const glm::vec2& v)
            {
                return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1);
            };

            size_t h1 = hashVec2(vertex.position);
            size_t h2 = hashVec3(vertex.color);


            // Combine all hashes
            return h1 ^ (h2 << 1);
        }
    };
}

