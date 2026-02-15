#include "platform/input/InputManager.h"
#include "camera/FirstPersonCamera.h"
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL_scancode.h>

namespace input
{
    InputManager::InputManager()
        : keyboard_state(nullptr),
          camera_mouse_button(SDL_BUTTON_LEFT),
          camera_control_active(false),
          mouse_sensitivity(0.1f),
          movement_speed(2.5f)
    {
        keyboard_state = SDL_GetKeyboardState(nullptr);
    }

    void InputManager::register_action(InputAction action, const ActionCallback& callback)
    {
        action_callbacks[action].push_back(callback);
    }

    void InputManager::unregister_action(InputAction action)
    {
        action_callbacks.erase(action);
    }

    void InputManager::trigger_action(InputAction action, float value)
    {
        auto it = action_callbacks.find(action);
        if (it != action_callbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback(value);
            }
        }
    }

    void InputManager::process_input(bool& is_running, camera::FirstPersonCamera* camera, double delta_time)
    {
        SDL_Event event;
        const ImGuiIO& io = ImGui::GetIO();

        // Process events from the OS
        while (SDL_PollEvent(&event))
        {
            // 1. Always pass events to ImGui first
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT)
            {
                is_running = false;
                continue;
            }

            // 2. Handle events for camera/game only if ImGui doesn't want them
            handle_event(event, camera);
        }

        // 3. Process continuous keyboard input only when camera control is active and ImGui doesn't want keyboard
        if (!io.WantCaptureKeyboard && camera_control_active)
        {
            process_keyboard_input(camera, delta_time);
            process_action_callbacks(delta_time);
        }
    }

    void InputManager::handle_event(const SDL_Event& event, camera::FirstPersonCamera* camera)
    {
        const ImGuiIO& io = ImGui::GetIO();
        const bool imgui_wants_mouse = io.WantCaptureMouse;
        const bool imgui_wants_keyboard = io.WantCaptureKeyboard;

        switch (event.type)
        {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if (!imgui_wants_mouse)
                {
                    if (event.button.button == camera_mouse_button)
                    {
                        camera_control_active = true;
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (event.button.button == camera_mouse_button)
                {
                    camera_control_active = false;
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
            {
                if (!imgui_wants_mouse && camera && camera_control_active)
                {
                    camera->process_mouse_movement(
                        event.motion.xrel,
                        event.motion.yrel
                    );

                    // Trigger action callbacks for mouse look
                    trigger_action(InputAction::LOOK_HORIZONTAL, static_cast<float>(event.motion.xrel));
                    trigger_action(InputAction::LOOK_VERTICAL, static_cast<float>(event.motion.yrel));
                }
                break;
            }

            case SDL_EVENT_MOUSE_WHEEL:
            {
                if (!imgui_wants_mouse && camera)
                {
                    camera->process_mouse_scroll(event.wheel.y);
                    trigger_action(InputAction::ZOOM, event.wheel.y);
                }
                break;
            }

            case SDL_EVENT_KEY_DOWN:
            {
                if (!imgui_wants_keyboard)
                {
                    // Handle specific key press events
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        //trigger_action(InputAction::PAUSE, 1.0f);
                    }
                    if (event.key.scancode == SDL_SCANCODE_F)
                    {
                        //trigger_action(InputAction::INTERACT, 1.0f);
                    }
                }
                break;
            }

            case SDL_EVENT_KEY_UP:
            {
                break;
            }

            default:
                break;
        }
    }

    void InputManager::process_keyboard_input(camera::FirstPersonCamera* camera, double delta_time)
    {
        if (!camera) return;

        keyboard_state = SDL_GetKeyboardState(nullptr);
        camera->process_keyboard(keyboard_state, static_cast<float>(delta_time));
    }

    void InputManager::process_action_callbacks(double delta_time)
    {
        keyboard_state = SDL_GetKeyboardState(nullptr);
        const auto dt = static_cast<float>(delta_time);

        // Process keyboard actions for callbacks
        if (keyboard_state[SDL_SCANCODE_W])
        {
            trigger_action(InputAction::MOVE_FORWARD, dt);
        }
        if (keyboard_state[SDL_SCANCODE_S])
        {
            trigger_action(InputAction::MOVE_BACKWARD, dt);
        }
        if (keyboard_state[SDL_SCANCODE_A])
        {
            trigger_action(InputAction::MOVE_LEFT, dt);
        }
        if (keyboard_state[SDL_SCANCODE_D])
        {
            trigger_action(InputAction::MOVE_RIGHT, dt);
        }
        if (keyboard_state[SDL_SCANCODE_SPACE] || keyboard_state[SDL_SCANCODE_E])
        {
            trigger_action(InputAction::MOVE_UP, dt);
        }
        if (keyboard_state[SDL_SCANCODE_LSHIFT] || keyboard_state[SDL_SCANCODE_Q])
        {
            trigger_action(InputAction::MOVE_DOWN, dt);
        }
    }

    bool InputManager::imgui_wants_mouse()
    {
        const ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }

    bool InputManager::imgui_wants_keyboard()
    {
        const ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }

    void InputManager::set_camera_mouse_button(const Uint8 button)
    {
        camera_mouse_button = button;
    }

    Uint8 InputManager::get_camera_mouse_button() const
    {
        return camera_mouse_button;
    }

    void InputManager::set_mouse_sensitivity(const float sensitivity)
    {
        mouse_sensitivity = sensitivity;
    }

    void InputManager::set_movement_speed(const float speed)
    {
        movement_speed = speed;
    }

    float InputManager::get_mouse_sensitivity() const
    {
        return mouse_sensitivity;
    }

    float InputManager::get_movement_speed() const
    {
        return movement_speed;
    }
} // namespace input