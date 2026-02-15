#pragma once
#include <functional>

#include "camera/FirstPersonCamera.h"
#include "enums/inputs/InputAction.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_stdinc.h"

namespace input
{
    using ActionCallback = std::function<void(float)>;

    class InputManager
    {
    public:
        InputManager();
        ~InputManager() = default;

        // Register a callback for a specific action
        void register_action(InputAction action, const ActionCallback& callback);

        // Unregister a callback for an action
        void unregister_action(InputAction action);

        // Trigger an action manually
        void trigger_action(InputAction action, float value = 1.0f);

        // Main input processing (call this every frame)
        void process_input(bool& is_running, camera::FirstPersonCamera* camera, double delta_time);

        // Check if ImGui wants to capture input
        static bool imgui_wants_mouse();
        static bool imgui_wants_keyboard();

        // Mouse button state for camera control
        void set_camera_mouse_button(Uint8 button);
        [[nodiscard]] Uint8 get_camera_mouse_button() const;

        // Sensitivity and speed settings
        void set_mouse_sensitivity(float sensitivity);
        void set_movement_speed(float speed);
        [[nodiscard]] float get_mouse_sensitivity() const;
        [[nodiscard]] float get_movement_speed() const;

    private:
        // Action callback map
        std::unordered_map<InputAction, std::vector<ActionCallback>> action_callbacks;

        // Input state
        const bool* keyboard_state;
        Uint8 camera_mouse_button;
        bool camera_control_active;

        // Settings
        float mouse_sensitivity;
        float movement_speed;

        // Helper methods
        void handle_event(const SDL_Event& event, camera::FirstPersonCamera* camera);
        void process_keyboard_input(camera::FirstPersonCamera* camera, double delta_time);
        void process_action_callbacks(double delta_time);

    };
}
