#include "camera/FirstPersonCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <numeric>

#include "structs/scene/CameraData.h"

namespace camera
{
    FirstPersonCamera::FirstPersonCamera(
        glm::vec3 position,
        float fov,
        float aspect_ratio,
        float near_plane,
        float far_plane
    ) : position(position),
        fov(fov),
        aspect_ratio(aspect_ratio),
        near_plane(near_plane),
        far_plane(far_plane),
        yaw(-90.0f),
        pitch(0.0f),
        movement_speed(3.5f),
        mouse_sensitivity(0.1f),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        right(glm::vec3(1.0f, 0.0f, 0.0f)),
        world_up(glm::vec3(0.0f, -1.0f, 0.0f))
    {
        update_camera_vectors();
    }

    glm::mat4 FirstPersonCamera::get_view_matrix() const
    {
        glm::mat4 view = glm::lookAt(position, position + front, up);
        return view;
    }

    glm::mat4 FirstPersonCamera::get_projection_matrix() const
    {
        glm::mat4 proj = glm::perspectiveZO(glm::radians(fov), aspect_ratio, near_plane, far_plane);
        proj[1][1] *= -1; // Flip Y for Vulkan
        return proj;
    }

    void FirstPersonCamera::process_keyboard(const bool* keyboard_state, float delta_time)
    {
        float velocity = movement_speed * delta_time;

        if (keyboard_state[SDL_SCANCODE_W]) {
            position += front * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_S]) {
            position -= front * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_A]) {
            position -= right * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_D]) {
            position += right * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_SPACE]) {
            position += world_up * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_LSHIFT]) {
            position -= world_up * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_Q]) {
            position -= world_up * velocity;
        }
        if (keyboard_state[SDL_SCANCODE_E]) {
            position += world_up * velocity;
        }
    }

    void FirstPersonCamera::process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch)
    {
        xoffset *= mouse_sensitivity;
        yoffset *= mouse_sensitivity;

        yaw -= xoffset;
        pitch -= yoffset;

        if (constrain_pitch) {
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        update_camera_vectors();
    }

    void FirstPersonCamera::process_mouse_scroll(float yoffset) {
        fov -= yoffset;
        if (fov < 1.0f) fov = 1.0f;
        if (fov > 120.0f) fov = 120.0f;
    }

    void FirstPersonCamera::set_position(const glm::vec3& pos) {
        position = pos;
    }

    void FirstPersonCamera::set_aspect_ratio(float ratio) {
        aspect_ratio = ratio;
    }

    void FirstPersonCamera::set_movement_speed(float speed) {
        movement_speed = speed;
    }

    void FirstPersonCamera::set_mouse_sensitivity(float sensitivity) {
        mouse_sensitivity = sensitivity;
    }

    void FirstPersonCamera::set_fov(float field_of_view) {
        fov = field_of_view;
    }

    glm::vec3 FirstPersonCamera::get_position() const {
        return position;
    }

    glm::vec3 FirstPersonCamera::get_front() const {
        return front;
    }

    glm::vec3 FirstPersonCamera::get_up() const {
        return up;
    }

    glm::vec3 FirstPersonCamera::get_right() const {
        return right;
    }

    float FirstPersonCamera::get_fov() const {
        return fov;
    }

    float FirstPersonCamera::get_yaw() const {
        return yaw;
    }

    float FirstPersonCamera::get_pitch() const {
        return pitch;
    }

    glm::vec3 FirstPersonCamera::get_htanfovxy_focal(float viewport_height) const
    {
        // Compute half-tangent of vertical FOV
        float fov_radians = glm::radians(fov);
        float htany = std::tan(fov_radians / 2.0f);

        // Compute half-tangent of horizontal FOV using aspect ratio
        float htanx = htany * aspect_ratio;

        // Compute focal length in pixels (based on viewport height)
        float focal = viewport_height / (2.0f * htany);

        return glm::vec3(htanx, htany, focal);
    }

    void FirstPersonCamera::update_camera_vectors() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(direction);
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
} // camera