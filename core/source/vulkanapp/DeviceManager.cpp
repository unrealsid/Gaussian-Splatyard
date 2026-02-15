#include "vulkanapp/DeviceManager.h"

#include <iostream>
#include "structs/EngineContext.h"
#include "vulkanapp/VulkanCleanupQueue.h"
#include "vulkanapp/feature_activator/VulkanFeatureActivator.h"

vulkanapp::DeviceManager::DeviceManager(EngineContext& engine_context): surface(nullptr), compute_queue(nullptr),
                                                                                                       graphics_queue(nullptr), present_queue(nullptr),
                                                                                                       vma_allocator(nullptr), engine_context(engine_context){ }
vulkanapp::DeviceManager::~DeviceManager()= default;

bool vulkanapp::DeviceManager::device_init()
{
    //Instance Creation
    // Create the disable feature struct
    VkValidationFeatureDisableEXT disables[] =
    {
        VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
    };
    
    vkb::InstanceBuilder instance_builder;
    auto instance_ret = instance_builder.
        set_minimum_instance_version(VK_API_VERSION_1_4)
        .use_default_debug_messenger()
        .add_validation_feature_disable(*disables)
        .enable_layer("VK_LAYER_KHRONOS_shader_object")
        .enable_extension(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)
        .enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
        .enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
        .request_validation_layers()
        .build();
    
    if (!instance_ret)
    {
        std::cout << instance_ret.error().message() << "\n";
        return false;
    }
    instance = instance_ret.value();
    engine_context.instance_dispatch_table = instance.make_table();

    //Activate device features
    VkPhysicalDeviceFeatures features = {};
    features.geometryShader = VK_FALSE;
    features.tessellationShader = VK_FALSE;
    features.largePoints = VK_TRUE;
    features.fillModeNonSolid = VK_TRUE;

    surface = engine_context.window_manager->create_surface_sdl3(instance_ret.value().instance, nullptr);

    vkb::PhysicalDeviceSelector phys_device_selector(instance);
    auto phys_device_ret = phys_device_selector
        .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
        .add_required_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)
        .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
        .add_required_extension(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)
        .add_required_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
        .add_required_extension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)
        .add_required_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
        .add_required_extension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)
        .add_required_extension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) 
        .add_required_extension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE3_EXTENSION_NAME)
        .add_required_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)
        .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
        .add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
        .add_required_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)
        //.add_required_extension(VK_KHR_MAINTENANCE_6_EXTENSION_NAME)
        .set_required_features(features)
        .set_surface(surface)
        .select();

    auto dynamic_rendering_features = VulkanFeatureActivator::create_dynamic_rendering_features();
    auto shader_object_features = VulkanFeatureActivator::create_shader_object_features();
    auto device_memory_features = VulkanFeatureActivator::create_physics_device_buffer_address();
    auto descriptorIndexingFeatures = VulkanFeatureActivator::create_physical_device_descriptor_indexing_features();
    auto synchronization2_features = VulkanFeatureActivator::create_synchronization2_features();
    auto vertex_input_dynamic_state_features = VulkanFeatureActivator::create_vertex_input_dynamic_state_features();
    auto dynamic_local_read_state_features = VulkanFeatureActivator::create_dynamic_rendering_local_read_features();
    
    if (!phys_device_ret)
    {
        std::cout << phys_device_ret.error().message() << "\n";
        return false;
    }
    const vkb::PhysicalDevice& p_device = phys_device_ret.value();

    vkb::DeviceBuilder device_builder{ p_device };
    auto device_ret = device_builder
        .add_pNext(&dynamic_rendering_features)
        .add_pNext(&shader_object_features)
        .add_pNext(&device_memory_features)
        .add_pNext(&descriptorIndexingFeatures)
        .add_pNext(&synchronization2_features)
        .add_pNext(&vertex_input_dynamic_state_features)
        .add_pNext(&dynamic_local_read_state_features)
        .build();
    
    if (!device_ret)
    {
        std::cout << device_ret.error().message() << "\n";
        return false;
    }

    device = device_ret.value();
    physical_device = p_device;
    engine_context.dispatch_table = device.make_table();
    
    return true;
}

bool vulkanapp::DeviceManager::init_queues()
{
    auto gq = device.get_queue(vkb::QueueType::graphics);
    if (!gq.has_value())
    {
        std::cout << "failed to get graphics queue: " << gq.error().message() << "\n";
        return false;
    }
    graphics_queue = gq.value();

    auto pq = device.get_queue(vkb::QueueType::present);
    if (!pq.has_value())
    {
        std::cout << "failed to get present queue: " << pq.error().message() << "\n";
        return false;
    }

    present_queue = pq.value();
    
    auto cq = device.get_queue(vkb::QueueType::compute);
    if (!cq.has_value())
    {
        std::cout << "failed to get compute queue: " << cq.error().message() << "\n";
        return false;
    }

    compute_queue = cq.value();
    
    return true;
}

void vulkanapp::DeviceManager::cleanup()
{
    if (vma_allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(vma_allocator);
        vma_allocator = VK_NULL_HANDLE;
    }

    if(instance.debug_messenger)
    {
        engine_context.instance_dispatch_table.destroyDebugUtilsMessengerEXT(instance.debug_messenger, nullptr);
    }

    engine_context.instance_dispatch_table.destroySurfaceKHR(surface, nullptr);
    
    //No corresponding Vk Bootstrapper function for destroy device
    vkDestroyDevice(device.device, nullptr);
    engine_context.instance_dispatch_table.destroyInstance(nullptr);
}
