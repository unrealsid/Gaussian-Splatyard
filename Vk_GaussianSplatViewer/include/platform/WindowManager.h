#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

struct WindowCreateParams;
struct EngineContext;

//Manages window functionality
namespace platform
{
    class WindowManager
    {
    public:
        WindowManager(EngineContext& engine_context);
        ~WindowManager();

        //Mouse coordinates clamped to the viewport
        int32_t local_mouse_x = 0;
        int32_t local_mouse_y = 0;

        int32_t last_mouse_x;
        int32_t last_mouse_y;
        
        int32_t mouse_delta_x;
        int32_t mouse_delta_y;

        SDL_Window* create_window_sdl3(const WindowCreateParams& window_create_params, bool resize = true);
        void destroy_window_sdl3() const;
        VkSurfaceKHR create_surface_sdl3(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;
        
        [[nodiscard]] SDL_Window* get_window() const;

        void update_mouse_position();
        double get_mouse_x() const { return mouse_x > 0 ? mouse_x : 0.0 ; }
        double get_mouse_y() const { return mouse_y > 0 ? mouse_y : 0.0 ; }

        //Clmaps mouse location to swapchain size
        bool get_local_mouse_xy(); 
        
        bool has_mouse_moved() const { return mouse_moved; }
        void reset_mouse_moved_flag() { mouse_moved = false; }
        void get_mouse_delta();
        void update_last_mouse_position();
        
        void update_window_dimensions();
        void setup_window_resize_callback();

        [[nodiscard]] uint32_t get_window_width() const { return m_window_width; }
        [[nodiscard]] uint32_t get_window_height() const { return m_window_height; }

    private:
        static bool sdl_window_event_watcher(void* userdata, SDL_Event* event);

        SDL_Window* window;
        EngineContext& engine_context;

        uint32_t m_window_width = 0;
        uint32_t m_window_height = 0;

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        bool mouse_moved = false;
    };
}
