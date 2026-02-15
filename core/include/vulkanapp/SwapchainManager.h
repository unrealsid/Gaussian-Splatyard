#pragma once

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <vector>

struct EngineContext;
struct EngineContext;

//Manages the swapchain
namespace vulkanapp
{
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct SwapchainImage
    {
        VkImage image;
        VkImageView image_view;
    };

    class SwapchainManager
    {
    public:
        explicit SwapchainManager(EngineContext& engine_context);
        ~SwapchainManager() = default;


        void initialize(VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface, uint32_t p_max_image_count, uint32_t window_width, uint32_t window_height);

        bool create_swapchain(uint32_t window_width, uint32_t window_height);

        bool recreate_swapchain(uint32_t window_width, uint32_t window_height);

        void cleanup();

        [[nodiscard]] SwapchainSupportDetails query_swapchain_support() const;
        [[nodiscard]] VkSwapchainKHR get_swapchain() const { return m_swapchain; }
        [[nodiscard]] VkFormat get_image_format() const { return m_image_format; }
        [[nodiscard]] VkExtent2D get_extent() const { return m_extent; }
        [[nodiscard]] const std::vector<SwapchainImage>& get_images() const { return m_images; }
        [[nodiscard]] uint32_t get_image_count() const { return static_cast<uint32_t>(m_images.size()); }


    private:
        [[nodiscard]] VkSurfaceFormatKHR select_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) const;

        [[nodiscard]] VkPresentModeKHR select_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) const;


        [[nodiscard]] VkExtent2D select_extent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t window_width,
        uint32_t window_height) const;


        bool create_image_views();

        void destroy_image_views();

        void destroy_swapchain();

        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;


        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<SwapchainImage> m_images;


        VkFormat m_image_format = VK_FORMAT_UNDEFINED;
        VkExtent2D m_extent = {0, 0};

        uint32_t max_image_count;

        EngineContext& engine_context;
        vkb::DispatchTable dispatch_table;
        vkb::InstanceDispatchTable instance_dispatch_table;
    };
}