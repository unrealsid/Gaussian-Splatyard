#include "vulkanapp/feature_activator/VulkanFeatureActivator.h"

VkPhysicalDeviceDynamicRenderingFeaturesKHR vulkanapp::VulkanFeatureActivator::create_dynamic_rendering_features()
{
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
    dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_features.pNext = nullptr;
    dynamic_rendering_features.dynamicRendering = VK_TRUE;

    return dynamic_rendering_features;
}

VkPhysicalDeviceShaderObjectFeaturesEXT vulkanapp::VulkanFeatureActivator::create_shader_object_features()
{
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features{};
    shader_object_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
    shader_object_features.pNext = nullptr;
    shader_object_features.shaderObject = VK_TRUE;

    return shader_object_features;
}

VkPhysicalDeviceBufferDeviceAddressFeatures vulkanapp::VulkanFeatureActivator::create_physics_device_buffer_address()
{
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = 
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        nullptr,
        VK_TRUE,
        VK_TRUE,
        VK_FALSE
    };

    return bufferDeviceAddressFeatures;
}

VkPhysicalDeviceDescriptorIndexingFeatures vulkanapp::VulkanFeatureActivator::create_physical_device_descriptor_indexing_features()
{
    VkPhysicalDeviceDescriptorIndexingFeatures features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features.runtimeDescriptorArray = VK_TRUE;
    features.descriptorBindingVariableDescriptorCount = VK_TRUE;

    return features;
}

VkPhysicalDeviceSynchronization2Features vulkanapp::VulkanFeatureActivator::create_synchronization2_features()
{
    VkPhysicalDeviceSynchronization2Features synchronization2Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES};
    synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    synchronization2Features.pNext = nullptr;
    synchronization2Features.synchronization2 = VK_TRUE;

    return synchronization2Features;
}

VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vulkanapp::VulkanFeatureActivator::create_vertex_input_dynamic_state_features()
{
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT};
    vertexInputDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
    vertexInputDynamicStateFeatures.pNext = nullptr;
    vertexInputDynamicStateFeatures.vertexInputDynamicState = VK_TRUE;

    return vertexInputDynamicStateFeatures;
}

VkPhysicalDeviceDynamicRenderingLocalReadFeatures vulkanapp::VulkanFeatureActivator::create_dynamic_rendering_local_read_features()
{
    VkPhysicalDeviceDynamicRenderingLocalReadFeatures dynamicRenderingFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
        .pNext = nullptr,
        .dynamicRenderingLocalRead = VK_TRUE
    };

    return dynamicRenderingFeatures;
}

VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR vulkanapp::VulkanFeatureActivator::create_shader_relaxed_extended_instruction_features()
{
    VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR,
        .pNext = nullptr,
        .shaderRelaxedExtendedInstruction = VK_TRUE
    };

    return features;
}

VkPhysicalDeviceVulkan13Features vulkanapp::VulkanFeatureActivator::create_vulkan_13_features()
{
    VkPhysicalDeviceVulkan13Features features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = nullptr,
        .shaderDemoteToHelperInvocation = VK_TRUE,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE
    };

    return features;
}


