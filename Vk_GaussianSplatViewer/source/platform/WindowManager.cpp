#include "platform/WindowManager.h"
#include <iostream>
#include "structs/EngineContext.h"
#include "structs/WindowCreateParams.h"
#include "vulkanapp/SwapchainManager.h"
#include "vulkanapp/DeviceManager.h"
#include "config/Config.inl"
#include <SDL3/SDL.h>

#include "vulkanapp/VulkanCleanupQueue.h"

platform::WindowManager::WindowManager(EngineContext& engine_context) : window(nullptr), engine_context(engine_context)
{
}

platform::WindowManager::~WindowManager()
{
    
}

SDL_Window* platform::WindowManager::create_window_sdl3(const WindowCreateParams& window_create_params, bool resize)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        // Handle error
        std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    Uint32 flags = SDL_WINDOW_VULKAN;
    if (resize)
    {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(window_create_params.window_title,
                           window_create_params.window_width,
                           window_create_params.window_height, flags);

    if (!window)
    {
        // Handle error
        std::cout << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    setup_window_resize_callback();

    return window;  
}

void platform::WindowManager::destroy_window_sdl3() const
{
    SDL_DestroyWindow(window);
    SDL_Quit();   
}

VkSurfaceKHR platform::WindowManager::create_surface_sdl3(VkInstance instance, VkAllocationCallbacks* allocator) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &surface))
    {
        std::cout << "Failed to create Vulkan surface: " << SDL_GetError() << std::endl;
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

SDL_Window* platform::WindowManager::get_window() const
{
    return window;
}

void platform::WindowManager::update_mouse_position()
{
    float new_x, new_y;
    SDL_GetMouseState(&new_x, &new_y);
    
    // Check if mouse has moved
    if (new_x != mouse_x || new_y != mouse_y)
    {
        mouse_moved = true;
        mouse_x = new_x;
        mouse_y = new_y;
    }
}

bool platform::WindowManager::get_local_mouse_xy()
{
    auto swapchain_extents = engine_context.swapchain_manager->get_extent();
    
    if (mouse_x > 0 && mouse_x < swapchain_extents.width)
    {
        local_mouse_x = mouse_x;
    }

    if (mouse_y > 0 && mouse_y < swapchain_extents.height)
    {
        local_mouse_y = mouse_y;
    }
    return true;
}

void platform::WindowManager::get_mouse_delta()
{
    mouse_delta_x = local_mouse_x - last_mouse_x;
    mouse_delta_y = local_mouse_y - last_mouse_y;
}

void platform::WindowManager::update_last_mouse_position()
{
    last_mouse_x = local_mouse_x;
    last_mouse_y = local_mouse_y;
}

void platform::WindowManager::update_window_dimensions()
{
    if (window != nullptr)
    {
        int width = 0, height = 0;
        SDL_GetWindowSize(window, &width, &height);
        m_window_width = static_cast<uint32_t>(width);
        m_window_height = static_cast<uint32_t>(height);
    }
}

void platform::WindowManager::setup_window_resize_callback()
{
    if (window != nullptr)
    {
        SDL_AddEventWatch(sdl_window_event_watcher, this);
    }
}

bool platform::WindowManager::sdl_window_event_watcher(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_EVENT_WINDOW_RESIZED)
    {
        auto* window_manager = static_cast<WindowManager*>(userdata);
        if (window_manager != nullptr)
        {
            window_manager->update_window_dimensions();
        }
    }
    return true;
}
