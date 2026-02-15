#include "renderer/Renderer.h"
#include <iostream>
#include <ostream>
#include "config/Config.inl"
#include "renderer/RenderPass.h"
#include "structs/EngineContext.h"

namespace core::rendering
{
    Renderer::Renderer(EngineContext& engine_context, const uint32_t p_max_frames_in_flight) :
        engine_context(engine_context),
        max_frames_in_flight(p_max_frames_in_flight),
        render_pass(nullptr)
    {
    }

    camera::FirstPersonCamera* Renderer::get_camera() const
    {
        return first_person_camera.get();
    }

    void Renderer::renderer_init()
    {
        std::cout << "Renderer Setup" << std::endl;

        init_vulkan();
        
        create_camera_and_buffer();
        create_model_matrices_buffer();
        engine_context.buffer_container->allocate_splat_render_params_buffer();

        render_pass = std::make_unique<RenderPass>(engine_context, 2);
        render_pass->renderpass_init();

        std::cout << "Done initializing the renderer";
    }

    void Renderer::renderer_update() const
    {
        render_pass->record_commands_and_draw();
    }
}
