#include "vulkanapp/SwapchainManager.h"

#include "structs/EngineContext.h"
#include <iostream>
#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace vulkanapp
{
    SwapchainManager::SwapchainManager(EngineContext& engine_context) : max_image_count(2),
                                                                        engine_context(engine_context)
    {
        dispatch_table = engine_context.dispatch_table;
        instance_dispatch_table = engine_context.instance_dispatch_table;
    }

    void SwapchainManager::initialize(VkPhysicalDevice physical_device, VkDevice device,
                                      VkSurfaceKHR surface, uint32_t p_max_image_count,
                                      uint32_t window_width, uint32_t window_height)
    {
        if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE || surface == VK_NULL_HANDLE)
        {
            throw std::invalid_argument("Invalid Vulkan handles provided to SwapchainManager");
        }

        m_physical_device = physical_device;
        m_device = device;
        m_surface = surface;
        max_image_count = p_max_image_count;

        if (!create_swapchain(window_width, window_height))
        {
            throw std::runtime_error("Failed to create swapchain");
        }
    }

    bool SwapchainManager::create_swapchain(uint32_t window_width, uint32_t window_height)
    {
        SwapchainSupportDetails swapchain_support = query_swapchain_support();

        if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty())
        {
            return false;
        }

        VkSurfaceFormatKHR surface_format = select_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = select_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = select_extent(swapchain_support.capabilities, window_width, window_height);

        m_image_format = surface_format.format;
        m_extent = extent;

        uint32_t image_count = swapchain_support.capabilities.minImageCount;
        // Request max_image_count images if possible
        if (max_image_count > image_count)
        {
            image_count = max_image_count;
        }

        if (swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support.capabilities.maxImageCount)
        {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = m_surface;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
        create_info.preTransform = swapchain_support.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        if (dispatch_table.createSwapchainKHR(&create_info, nullptr, &m_swapchain) != VK_SUCCESS)
        {
            return false;
        }

        uint32_t swapchain_image_count = 0;
        dispatch_table.getSwapchainImagesKHR(m_swapchain, &swapchain_image_count, nullptr);

        swapchain_image_count = std::min(swapchain_image_count, max_image_count);

        std::vector<VkImage> swapchain_images(swapchain_image_count);
        dispatch_table.getSwapchainImagesKHR(m_swapchain, &swapchain_image_count, swapchain_images.data());

        m_images.resize(swapchain_image_count);
        for (uint32_t i = 0; i < swapchain_image_count; ++i)
        {
            m_images[i].image = swapchain_images[i];
            m_images[i].image_view = VK_NULL_HANDLE;
        }

        return create_image_views();
    }

    bool SwapchainManager::recreate_swapchain(uint32_t window_width, uint32_t window_height)
    {
        if (window_width == 0 || window_height == 0)
        {
            return false;
        }

        dispatch_table.deviceWaitIdle();

        destroy_image_views();
        destroy_swapchain();

        return create_swapchain(window_width, window_height);
    }

    void SwapchainManager::cleanup()
    {
        destroy_image_views();
        destroy_swapchain();

        m_physical_device = VK_NULL_HANDLE;
        m_device = VK_NULL_HANDLE;
        m_surface = VK_NULL_HANDLE;
    }

    SwapchainSupportDetails SwapchainManager::query_swapchain_support() const
    {
        SwapchainSupportDetails details;

        instance_dispatch_table.getPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &details.capabilities);

        uint32_t format_count = 0;
        instance_dispatch_table.getPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            instance_dispatch_table.getPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count,
                                                     details.formats.data());
        }

        uint32_t present_mode_count = 0;
        instance_dispatch_table.getPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count,
                                                      nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            instance_dispatch_table.getPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count,
                                                          details.present_modes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR SwapchainManager::select_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) const
    {
        for (const auto& format : available_formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return format;
                }
        }

        return available_formats[0];
    }

    VkPresentModeKHR SwapchainManager::select_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) const
    {
        for (const auto& present_mode : available_present_modes)
        {
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapchainManager::select_extent(const VkSurfaceCapabilitiesKHR& capabilities,
                                               uint32_t window_width,
                                               uint32_t window_height) const {

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actual_extent{window_width, window_height};

        actual_extent.width = std::clamp(actual_extent.width,
                                         capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width);

        actual_extent.height = std::clamp(actual_extent.height,
                                          capabilities.minImageExtent.height,
                                          capabilities.maxImageExtent.height);

        return actual_extent;
    }

    bool SwapchainManager::create_image_views() {
        for (auto& image : m_images) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = image.image;
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = m_image_format;

            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            if (dispatch_table.createImageView(&create_info, nullptr, &image.image_view) != VK_SUCCESS) {
                return false;
            }
        }

        return true;
    }

    void SwapchainManager::destroy_image_views()
    {
        for (auto& image : m_images)
        {
            if (image.image_view != VK_NULL_HANDLE) {
                dispatch_table.destroyImageView(image.image_view, nullptr);
                image.image_view = VK_NULL_HANDLE;
            }
        }
    }

    void SwapchainManager::destroy_swapchain()
    {
        if (m_swapchain != VK_NULL_HANDLE)
        {
            dispatch_table.destroySwapchainKHR(m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }

        m_images.clear();
        m_image_format = VK_FORMAT_UNDEFINED;
        m_extent = {0, 0};
    }
}
