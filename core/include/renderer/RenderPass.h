#pragma once

#include "renderer/subpasses/ComputePass.h"
#include "Subpass.h"
#include "camera/FirstPersonCamera.h"
#include "structs/Vk_Image.h"
#include "common/Types.h"
#include "structs/EngineRenderTargets.h"
#include "structs/scene/CameraData.h"
struct EngineContext;

namespace vulkanapp
{
    class SwapchainManager;
    class DeviceManager;
}

namespace core::rendering
{
    class GPU_BufferContainer;
    class ComputePass;

    class RenderPass
    {
    public:
        RenderPass(EngineContext& engine_context, uint32_t max_frames_in_flight = 2);
        void allocate_command_buffers();
        void renderpass_init();

        void init_subpasses();

        [[nodiscard]] uint32_t get_max_frames_in_flight() const { return max_frames_in_flight; }

        VkCommandBuffer* get_command_buffer(uint32_t image_id);

        [[nodiscard]] VkCommandPool get_command_pool() const { return command_pool; }

        [[nodiscard]] std::vector<VkFence>& get_compute_fences() { return compute_in_flight_fences; }

        void create_command_pool();
        void reset_command_pool();

        void allocate_command_buffer(uint32_t image);

        void create_depth_stencil_image();

        void reset_subpass_command_buffers();
        void recreate_swapchain();
        void create_renderpass_resources();

        bool draw_frame(uint32_t image_index);

        void record_subpasses(uint32_t image_index);
        void record_compute_pass_and_submit(uint32_t image_index, const vkb::DispatchTable& dispatch_table);
        void recreate_render_resources();

        void record_commands_and_draw();
        void cleanup();

    private:
        std::unique_ptr<ComputePass> compute_pass;
        std::vector<std::unique_ptr<Subpass>> subpasses;
        std::vector<VkCommandBuffer> command_buffers;
        std::vector<VkCommandBuffer> compute_command_buffers;

        vulkanapp::SwapchainManager* swapchain_manager{};
        vulkanapp::DeviceManager* device_manager{};

        std::vector<VkSemaphore> available_semaphores;
        std::vector<VkSemaphore> finished_semaphores;
        std::vector<VkSemaphore> compute_finished_semaphores;

        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> image_in_flight;
        std::vector<VkFence> compute_in_flight_fences;

        EngineContext& engine_context;

        VkCommandPool command_pool;

        uint32_t max_frames_in_flight{};

        size_t current_frame = 0;

        //Stores camera data on the cpu
        CameraData camera_data{};

        //References a camera
        camera::FirstPersonCamera* camera;

        //Subpass dependent shader objects
        //Individual subpasses initialize their own subpass shader objects if needed
        SubpassShaderList subpass_shader_objects;

        //Stores a reference to the buffer container
        GPU_BufferContainer* buffer_container;

        EngineRenderTargets engine_render_targets;

        bool create_sync_objects();
        void set_new_camera_aspect_ratio() const;
        void map_camera_data();

        //Maps cpu -> gpu data
        void map_cpu_data();
    };
}
