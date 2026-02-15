#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>
#include "Vk_Image.h"

//Used to store and pass info for render attachment creation.

struct RenderingAttachmentInfoData
{
    //Which view to use?
    VkImageView image_view{};

    //What is the initial surface value?
    VkClearValue surface_clear_value{};

    //What image layout to use
    VkImageLayout image_layout{};

    //Specify how to load the image -- By default it's clear
    VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;

    //Specify how to store the image -- By default store
    VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
};
