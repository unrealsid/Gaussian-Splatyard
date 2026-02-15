#include "core/Engine.h"
#include <chrono>

#include "3d/Scene.h"
#include "vulkanapp/VulkanCleanupQueue.h"
#include "config/Config.inl"
#include "renderer/Renderer.h"
#include "renderer/RenderPass.h"
#include "structs/WindowCreateParams.h"

void core::Engine::create_window() const
{
    engine_context->window_manager = std::make_unique<platform::WindowManager>(*engine_context);
    WindowCreateParams window_create_params = { window_width, window_height, window_title };
    engine_context->window_manager->create_window_sdl3(window_create_params, true);
}

void core::Engine::create_renderer() const
{
    engine_context->renderer = std::make_unique<rendering::Renderer>(*engine_context);
    engine_context->renderer->renderer_init();
}

void core::Engine::create_ui_and_input() const
{
    //Init UI Action manager
    engine_context->ui_action_manager = std::make_unique<ui::UIActionManager>();

    // Initialize input manager
    engine_context->input_manager = std::make_unique<input::InputManager>();
    engine_context->input_manager->set_camera_mouse_button(SDL_BUTTON_RIGHT);
}

void core::Engine::create_buffer_container() const
{
    engine_context->buffer_container = std::make_unique<core::rendering::GPU_BufferContainer>(*engine_context);
}

void core::Engine::create_scene() const
{
    entity_3d::Scene scene(*engine_context);
    scene.populate_scene();
}

void core::Engine::create_cleanup() const
{
    engine_context->renderer->cleanup_init();
    vulkanapp::VulkanCleanupQueue::push_cleanup_function(CLEANUP_FUNCTION(engine_context->window_manager->destroy_window_sdl3()));
}

void core::Engine::init()
{
    engine_context = std::make_unique<EngineContext>();
    
    create_window();
    create_ui_and_input();
    create_buffer_container();
    create_renderer();
    create_cleanup();

    //Populate the scene after all Vulkan resources have been created
    create_scene();
}

void core::Engine::process_input(bool& is_running, camera::FirstPersonCamera* camera, double delta_time) const
{
    engine_context->input_manager->process_input(is_running, camera, delta_time);
}

void core::Engine::process_ui_actions() const
{
    engine_context->ui_action_manager->process_queued_actions();
}

void core::Engine::run() const
{
    bool is_running = true;
   
    auto previous_time = std::chrono::high_resolution_clock::now();
    auto camera = engine_context->renderer->get_camera();

    while (is_running)
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - previous_time;
        double delta_time = elapsed.count();

        process_input(is_running, camera, delta_time);

        process_ui_actions();

        // update();

        // draw();
        engine_context->renderer->renderer_update();

        previous_time = current_time;
    }
}

void core::Engine::cleanup()
{
    vulkanapp::VulkanCleanupQueue::flush();
}
