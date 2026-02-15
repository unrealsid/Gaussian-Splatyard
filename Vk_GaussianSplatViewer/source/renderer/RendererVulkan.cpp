

#include "renderer/Renderer.h"
#include "config/Config.inl"
#include "structs/EngineContext.h"
#include "structs/geometry/Vertex2D.h"
#include "vulkanapp/VulkanCleanupQueue.h"
#include "vulkanapp/utils/MemoryUtils.h"
#include "renderer/GPU_BufferContainer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/RenderPass.h"

namespace core::rendering
{
    void Renderer::init_vulkan()
    {
        create_device();
        create_swapchain();
        utils::MemoryUtils::create_vma_allocator(*engine_context.device_manager);
    }

    void Renderer::create_swapchain() const
    {
        engine_context.swapchain_manager = std::make_unique<vulkanapp::SwapchainManager>(engine_context);
        engine_context.swapchain_manager->initialize(engine_context.device_manager->get_physical_device(),
                                                      engine_context.device_manager->get_device(),
                                                      //Windowing
                                                      engine_context.device_manager->get_surface(),
                                                      2,
                                                      engine_context.window_manager->get_window_width(),
                                                      engine_context.window_manager->get_window_height());
    }

    void Renderer::create_device() const
    {
        engine_context.device_manager = std::make_unique<vulkanapp::DeviceManager>(engine_context);
        engine_context.device_manager->device_init();
        engine_context.device_manager->init_queues();
    }

    void Renderer::cleanup_init() const
    {
        // Pushing in the order we want them to be destroyed in REVERSE
        // LIFO means the last one pushed is the first one executed.

        // 5. Device manager should be destroyed last (contains VMA allocator and VkDevice)
        vulkanapp::VulkanCleanupQueue::push_cleanup_function(CLEANUP_FUNCTION(engine_context.device_manager->cleanup()));

        // 4. Swapchain
        vulkanapp::VulkanCleanupQueue::push_cleanup_function(CLEANUP_FUNCTION(engine_context.swapchain_manager->cleanup()));

        // 2. Render pass cleanup (includes destroying sync objects, command pool, depth image)
        vulkanapp::VulkanCleanupQueue::push_cleanup_function(CLEANUP_FUNCTION(render_pass->cleanup()));

        // 1. Wait for idle must be the VERY FIRST thing we do when flushing the queue.
        // It's already called inside render_pass->cleanup(), but we can add it explicitly as the last push (first to execute)
        // just to be sure.
        vulkanapp::VulkanCleanupQueue::push_cleanup_function([this]() { engine_context.dispatch_table.deviceWaitIdle(); });
    }

    RenderPass* Renderer::get_render_pass() const
    {
        return render_pass.get();
    }

    void Renderer::create_camera_and_buffer()
    {
        auto swapchain_manager = engine_context.swapchain_manager.get();
        first_person_camera = std::make_unique<camera::FirstPersonCamera>(glm::vec3(0.0f, 0.0f, 3.0f), 45.0f,
                                                                          static_cast<float>(swapchain_manager->get_extent().width ) / static_cast<float>(swapchain_manager->get_extent().height));

        engine_context.buffer_container->allocate_camera_buffer(*first_person_camera);
    }

    void Renderer::create_model_matrices_buffer() const
    {
        engine_context.buffer_container->allocate_model_matrices_buffer();
    }

    void Renderer::cleanup()
    {
        vulkanapp::VulkanCleanupQueue::flush();
    }
}
