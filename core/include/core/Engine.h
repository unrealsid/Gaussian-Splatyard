#pragma once

#include <memory>
#include "structs/EngineContext.h"

struct GaussianSurface;

//Core engine class
namespace core
{
    class Engine
    {
    public:
        void init();

        void run() const;
        void cleanup();

    private:
        //Stored here pass this as a pointer or reference wherever needed
        std::unique_ptr<EngineContext> engine_context;

        void create_window() const;
        void create_renderer() const;
        void create_ui_and_input() const;
        void create_buffer_container() const;

        void create_scene() const;

        //Orders and stores cleanup function for resource clear
        void create_cleanup() const;

        void process_input(bool& is_running,
                             camera::FirstPersonCamera* camera,
                             double delta_time) const;

        void process_ui_actions() const;
    };
}
