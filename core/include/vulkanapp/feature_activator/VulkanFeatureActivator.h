#pragma once

#include <vulkan/vulkan.h>

namespace vulkanapp
{
    class VulkanFeatureActivator
    {
    public:
        static VkPhysicalDeviceDynamicRenderingFeaturesKHR create_dynamic_rendering_features();
        static VkPhysicalDeviceShaderObjectFeaturesEXT create_shader_object_features();
        static VkPhysicalDeviceBufferDeviceAddressFeatures create_physics_device_buffer_address();
        static VkPhysicalDeviceDescriptorIndexingFeatures create_physical_device_descriptor_indexing_features();
        static VkPhysicalDeviceSynchronization2Features create_synchronization2_features();
        static VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT create_vertex_input_dynamic_state_features();
        static VkPhysicalDeviceDynamicRenderingLocalReadFeatures create_dynamic_rendering_local_read_features();
        static VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR create_shader_relaxed_extended_instruction_features();
        static VkPhysicalDeviceVulkan13Features create_vulkan_13_features();
    };
}