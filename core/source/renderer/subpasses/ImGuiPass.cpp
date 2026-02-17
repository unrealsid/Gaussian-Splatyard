#include "renderer/subpasses/ImGuiPass.h"
#include "structs/EngineContext.h"
#include "vulkanapp/DeviceManager.h"
#include "vulkanapp/SwapchainManager.h"
#include "structs/scene/PushConstantBlock.h"
#include "common/Types.h"
#include <iostream>

#include "structs/scene/SplatRenderingParams.h"
#include "vulkanapp/utils/ImageUtils.h"
#include "vulkanapp/utils/RenderUtils.h"


namespace core::rendering
{
    ImGuiPass::ImGuiPass(EngineContext& engine_context, uint32_t max_frames_in_flight)
        : Subpass(engine_context, max_frames_in_flight)
    {

    }

    ImGuiPass::~ImGuiPass()
    {
        ImGuiPass::cleanup();
    }

    void ImGuiPass::frame_pre_recording()
    {

    }

    void ImGuiPass::subpass_init(SubpassShaderList& subpass_shaders, GPU_BufferContainer& buffer_container)
    {
        init_imgui();

        engine_context.ui_action_manager->register_action(
            UIAction::SORT_SPLATS,
             [this, &buffer_container]()
             {
                 const auto& indices = engine_context.splat_loader->sort_gaussians(*(engine_context.renderer->get_camera()));
                 buffer_container.map_splat_indices(indices);
             });


    }

    void ImGuiPass::render_target_init(EngineRenderTargets& render_targets)
    {
        extents = render_targets.swapchain_extent;
    }

    void ImGuiPass::init_imgui()
    {
        // Create Descriptor Pool for ImGui
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        if (engine_context.dispatch_table.createDescriptorPool(&pool_info, nullptr, &imgui_pool) != VK_SUCCESS)
        {
            std::cerr << "Failed to create ImGui descriptor pool" << std::endl;
            return;
        }

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL3_InitForVulkan(engine_context.window_manager->get_window());
        ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* user_data)
        {
            auto instance = static_cast<VkInstance>(user_data);
            return vkGetInstanceProcAddr(instance, function_name);
        }, engine_context.instance_dispatch_table.instance);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device_manager->get_instance().instance;
        init_info.PhysicalDevice = device_manager->get_physical_device().physical_device;
        init_info.Device = device_manager->get_device().device;
        init_info.QueueFamily = device_manager->get_device().get_queue_index(vkb::QueueType::graphics).value();
        init_info.Queue = device_manager->get_graphics_queue();
        init_info.DescriptorPool = imgui_pool;
        init_info.MinImageCount = max_frames_in_flight;
        init_info.ImageCount = swapchain_manager->get_image_count();
        init_info.UseDynamicRendering = true;
        init_info.UseShaderObjects = true;

        ImGui_ImplVulkan_Init(&init_info);
    }

    void ImGuiPass::map_splat_rendering_params()
    {
        auto buffer_container = engine_context.buffer_container.get();
        memcpy(buffer_container->splat_render_params_buffer.allocation_info.pMappedData, &splat_rendering_params, sizeof(SplatRenderingParams));
    }


    void ImGuiPass::record_commands(VkCommandBuffer* command_buffer, uint32_t image_index, SubpassShaderList& subpass_shaders,
                                    GPU_BufferContainer& buffer_container, EngineRenderTargets& render_targets, const std::vector<Renderable>& renderables)
    {
        //Create the color attachments for this pass -- decides what gets rendered
        auto color_attachments = utils::RenderUtils::create_color_attachments(
        {
            {
                render_targets.swapchain_images[image_index].image_view,
                {0.0f, 0.0f, 0.0f, 1.0f},
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // Don't clear what GeometryPass did
                VK_ATTACHMENT_LOAD_OP_LOAD
            }
        });//vector
        
        depth_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_attachment_info.imageView = VK_NULL_HANDLE;

        begin_rendering(color_attachments);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        // ImGui::Text("Gaussian Splat Loader");
        // {
        //     bool show_demo_window = true;
        //     bool show_another_window = false;
        //     ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        //     static float f = 0.0f;
        //     static int counter = 0;
        //
        //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        //
        //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //     ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        //     ImGui::Checkbox("Another Window", &show_another_window);
        //
        //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
        //
        //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);
        //     ImGui::End();
        // }


        ImGui::Begin("Gaussian Splat Loader");

        ImGui::Text("Load Gaussian Splat/PointCloud");

        static char text_buffer[256] = "";
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##filepath", text_buffer, IM_ARRAYSIZE(text_buffer));
        ImGui::PopItemWidth();

        if (ImGui::Button("Load File", ImVec2(-1, 0)))
        {
            engine_context.ui_action_manager->queue_string_action(UIAction::ALLOCATE_SPLAT_MEMORY, text_buffer);
        }

        if (ImGui::Button("Sort Splats", ImVec2(-1, 0)))
        {
            engine_context.ui_action_manager->queue_action(UIAction::SORT_SPLATS);
        }

        static float sigma = 0.0f;            // 0 -> 1000 (step 100)
        static float v_i = 0.0f;              // 0 -> 5
        static float depth_multiplier = 0.0f; // 0 -> 50
        static float power = 0.0f;            // 0 -> 10

        ImGui::Separator();
        ImGui::Text("Gaussian Parameters");

        // Sigma: stepped slider
        ImGui::SliderFloat("Sigma", &sigma, 0.0f, 1000.0f, "%.0f");
        //sigma = std::round(sigma / 100.0f) * 100.0f; // enforce 100-step increments

        // v_i
        ImGui::SliderFloat("v_i", &v_i, 0.0f, 5.0f, "%.3f");

        // Depth Multiplier
        ImGui::SliderFloat("Depth Multiplier", &depth_multiplier, 0.0f, 50.0f, "%.2f");

        // Power
        ImGui::SliderFloat("Power", &power, 0.0f, 100.0f, "%.2f");

        splat_rendering_params.rendering_params = { sigma, v_i, depth_multiplier, power };
        map_splat_rendering_params();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *command_buffer);

        end_rendering();
    }

    void ImGuiPass::cleanup()
    {
        if (imgui_pool != VK_NULL_HANDLE)
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            engine_context.dispatch_table.destroyDescriptorPool(imgui_pool, nullptr);
            imgui_pool = VK_NULL_HANDLE;
        }
    }
}
