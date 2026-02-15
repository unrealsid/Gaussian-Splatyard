#pragma once

#include <memory>
#include <span>

#include "Vk_Image.h"
#include "vulkanapp/SwapchainManager.h"

struct EngineRenderTargets
{
    //This is managed in the swapchain manager. Adding it here for convenience
    std::span<const vulkanapp::SwapchainImage> swapchain_images;
    VkExtent2D swapchain_extent;

    //Stores depth image
    std::unique_ptr<Vk_Image> depth_stencil_image;

    //Images storing transparency data
    std::unique_ptr<Vk_Image> accumulation_image;
    std::unique_ptr<Vk_Image> revealage_image;
};
