#pragma once

#include <glm/glm.hpp>

namespace camera
{
    class FirstPersonCamera
    {
    public:
        FirstPersonCamera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
            float fov = 45.0f,
            float aspect_ratio = 16.0f / 9.0f,
            float near_plane = 0.01f,
            float far_plane = 100.0f
        );

        // Get view matrix
        [[nodiscard]] glm::mat4 get_view_matrix() const;

        // Get projection matrix
        [[nodiscard]] glm::mat4 get_projection_matrix() const;

        // Process keyboard input
        void process_keyboard(const bool* keyboard_state, float delta_time);

        // Process mouse movement
        void process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch = true);

        // Process mouse scroll for FOV zoom
        void process_mouse_scroll(float yoffset);

        void set_position(const glm::vec3& pos);
        void set_aspect_ratio(float ratio);
        void set_movement_speed(float speed);
        void set_mouse_sensitivity(float sensitivity);
        void set_fov(float field_of_view);

        // Getters
        [[nodiscard]] glm::vec3 get_position() const;
        [[nodiscard]] glm::vec3 get_front() const;
        [[nodiscard]] glm::vec3 get_up() const;
        [[nodiscard]] glm::vec3 get_right() const;
        [[nodiscard]] float get_fov() const;
        [[nodiscard]] float get_yaw() const;
        [[nodiscard]] float get_pitch() const;
        [[nodiscard]] glm::vec3 get_htanfovxy_focal(float viewport_height) const;


    private:
        void update_camera_vectors();

        // Camera attributes
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 world_up;

        // Euler angles
        float yaw;
        float pitch;

        // Projection parameters
        float fov;
        float aspect_ratio;
        float near_plane;
        float far_plane;

        // Camera options
        float movement_speed;
        float mouse_sensitivity;
    };
}
