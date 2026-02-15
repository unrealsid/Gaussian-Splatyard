#pragma once

#include <memory>
#include "renderer/RenderPass.h"

namespace camera
{
    class FirstPersonCamera;
}

struct EngineContext;
struct WindowCreateParams;

namespace core::rendering
{
    class RenderPass;

    class Renderer
    {
    public:
        explicit Renderer(EngineContext& engine_context, uint32_t p_max_frames_in_flight = 2);

        [[nodiscard]] camera::FirstPersonCamera* get_camera() const;

        void renderer_init();
        void renderer_update() const;

        void cleanup_init() const;

        [[nodiscard]] RenderPass* get_render_pass() const;

        //Creates a first-person camera and associated Vulkan resources
        void create_camera_and_buffer();

        //Creates model matrices and related Vulkan buffer
        void create_model_matrices_buffer() const;

        std::vector<Renderable>& get_renderables() { return renderables; }

    private:
        EngineContext& engine_context;

        //How many images are we using for a single frame?
        uint32_t max_frames_in_flight;

        std::unique_ptr<camera::FirstPersonCamera> first_person_camera;

        std::unique_ptr<RenderPass> render_pass;

        //simply acts as a view for all items that can be rendered
        std::vector<Renderable> renderables;

        void init_vulkan();

        void create_swapchain() const;
        void create_device() const;

        void cleanup();
    };
}
