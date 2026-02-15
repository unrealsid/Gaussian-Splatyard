#include "vulkanapp/utils/ImageUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vma/vk_mem_alloc.h>
#include <VkBootstrapDispatch.h>
#include "vulkanapp/utils/MemoryUtils.h"
#include "structs/GPU_Buffer.h"
#include "vulkanapp/DeviceManager.h"
#include "structs/EngineContext.h"

LoadedImageData utils::ImageUtils::load_image_data(const std::string& filePath, int desired_channels)
{
    LoadedImageData imageData;

    int tempOriginalChannels; // Used to store the original channels from the file
    
    imageData.pixels = stbi_load(filePath.c_str(), &imageData.width, &imageData.height, &tempOriginalChannels, desired_channels);

    imageData.channels = desired_channels; // Store the number of channels we requested
    imageData.original_channels = tempOriginalChannels; // Store the original channels from the file

    if (!imageData.pixels)
    {
        std::cerr << "Failed to load image file: " << filePath << " - " << stbi_failure_reason() << std::endl;
        // Return an empty/invalid struct if loading failed
        imageData.width = 0;
        imageData.height = 0;
        imageData.channels = 0;
        imageData.original_channels = 0;
    }
    else
    {
        std::cout << "Loaded image: " << filePath << " (" << imageData.width << "x" << imageData.height << ", " << imageData.channels << " channels)" << std::endl;
    }

    return imageData;
}

Vk_Image utils::ImageUtils::create_texture_image(EngineContext& engine_context, VkCommandPool command_pool, const LoadedImageData& image_data)
{
    if (image_data.pixels)
    {
        VkDeviceSize imageSize = image_data.width * image_data.height * image_data.channels;
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

        auto device_manager = engine_context.device_manager.get();
        
        //allocate temporary buffer for holding texture data to upload
        //create staging buffer for image
        GPU_Buffer stagingImageBuffer;

        MemoryUtils::create_buffer(engine_context.dispatch_table,
                                     device_manager->get_allocator(), imageSize,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VMA_MEMORY_USAGE_AUTO,
                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                     stagingImageBuffer);
    
        void* data;
        vmaMapMemory(device_manager->get_allocator(), stagingImageBuffer.allocation, &data);
        memcpy(data, image_data.pixels, imageSize);
        vmaUnmapMemory(device_manager->get_allocator(), stagingImageBuffer.allocation);

        //Create image on the gpu
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(image_data.width);
        imageExtent.height = static_cast<uint32_t>(image_data.height);
        imageExtent.depth = 1;

        VkImageCreateInfo imgIfo = image_create_info(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        //allocate and create the image
        VmaAllocationCreateInfo imgAllocInfo = {};
        imgAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        
        Vk_Image textureImage;
        vmaCreateImage(device_manager->get_allocator(), &imgIfo, &imgAllocInfo, &textureImage.image, &textureImage.allocation, &textureImage.allocation_info);

        //Copy image to device Memory
        copy_image(engine_context, device_manager->get_graphics_queue(), command_pool, stagingImageBuffer, textureImage, imageSize, imageExtent);

        create_image_sampler(engine_context.dispatch_table, textureImage, VK_FILTER_LINEAR);

        create_image_view(engine_context.dispatch_table, textureImage, imageFormat);
        
        return textureImage;
    }

    return {};
}

VkImageCreateInfo utils::ImageUtils::image_create_info(VkFormat image_format, VkImageUsageFlags image_usage_flags, VkExtent3D image_extent)
{
    VkImageCreateInfo imgInfo = {};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.pNext = nullptr;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = image_format;
    imgInfo.extent = image_extent;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = image_usage_flags;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imgInfo.queueFamilyIndexCount = 0;
    imgInfo.pQueueFamilyIndices = nullptr;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.flags = 0;

    return imgInfo;
}

void utils::ImageUtils::copy_image(EngineContext& engine_context, VkQueue queue, VkCommandPool command_pool, GPU_Buffer srcBuffer, Vk_Image
                                  dstImage, VkDeviceSize size, VkExtent3D extend)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    engine_context.dispatch_table.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    engine_context.dispatch_table.beginCommandBuffer(commandBuffer, &beginInfo);

    //Layout transition
    //1. convert image to Transfer dst. Vk_Image now ready to receive data
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier2 imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

    imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toTransfer.image = dstImage.image;
    imageBarrier_toTransfer.subresourceRange = range;

    imageBarrier_toTransfer.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    imageBarrier_toTransfer.srcAccessMask = 0;
    imageBarrier_toTransfer.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier_toTransfer;

    //barrier the image into the transfer-receive layout
    //1.1 barrier
    engine_context.dispatch_table.cmdPipelineBarrier2(commandBuffer, &dependencyInfo);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = extend;

    //2. copy the buffer into the image
    engine_context.dispatch_table.cmdCopyBufferToImage(commandBuffer, srcBuffer.buffer, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    //3. Put barrier for image after copy
    VkImageMemoryBarrier2 imageBarrier_toReadable = imageBarrier_toTransfer;

    imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageBarrier_toReadable.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    imageBarrier_toReadable.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    imageBarrier_toReadable.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    dependencyInfo.pImageMemoryBarriers = &imageBarrier_toReadable;

    //barrier the image into the shader readable layout
    engine_context.dispatch_table.cmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    
    engine_context.dispatch_table.endCommandBuffer(commandBuffer);

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = commandBuffer;

    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    engine_context.dispatch_table.queueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE);
    engine_context.dispatch_table.queueWaitIdle(queue);

    engine_context.dispatch_table.freeCommandBuffers(command_pool, 1, &commandBuffer);

    vmaDestroyBuffer(engine_context.device_manager->get_allocator(), srcBuffer.buffer, srcBuffer.allocation);
}

void utils::ImageUtils::copy_image_to_buffer(EngineContext& render_context, Vk_Image src_image, GPU_Buffer& dst_buffer, VkCommandBuffer cmd_buffer, VkOffset3D image_offset)
{
    auto dispatch_table = render_context.dispatch_table;
    
    // Transition image for transfer
    ImageUtils::image_layout_transition(
        cmd_buffer,
        src_image.image,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    );

    //Copy the image to the buffer
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = image_offset;
    auto extents = render_context.swapchain_manager->get_extent();
    copy_region.imageExtent = {extents.width, extents.height, 1};
    
    dispatch_table.cmdCopyImageToBuffer(
        cmd_buffer,
        src_image.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst_buffer.buffer,
        1,
        &copy_region
    );

    //Add a barrier for the image 
    ImageUtils::image_layout_transition(
        cmd_buffer,
        src_image.image,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_2_TRANSFER_READ_BIT,
        0,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    );

    //And add a buffer for the buffer before it is read on the cpu
    VkBufferMemoryBarrier2 buffer_barrier{};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.buffer = dst_buffer.buffer;
    buffer_barrier.offset = 0;
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.bufferMemoryBarrierCount = 1;
    dependencyInfo.pBufferMemoryBarriers = &buffer_barrier;

    render_context.dispatch_table.cmdPipelineBarrier2(cmd_buffer, &dependencyInfo);
}

void utils::ImageUtils::create_image_sampler(const vkb::DispatchTable& disp, Vk_Image& image, VkFilter filter)
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = filter;
    samplerCreateInfo.minFilter = filter;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    disp.createSampler(&samplerCreateInfo, nullptr, &image.sampler);
}

void utils::ImageUtils::create_image_view(const vkb::DispatchTable& disp, Vk_Image& image, VkFormat format, VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = nullptr;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = { aspect_flags, 0, 1, 0, 1 };
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.image = image.image;
    disp.createImageView(&viewCreateInfo, nullptr, &image.view);
}

void  utils::ImageUtils::image_layout_transition(VkCommandBuffer command_buffer, VkImage image,
    VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 src_access_mask,
    VkAccessFlags2 dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout,
    const VkImageSubresourceRange& subresource_range)
{
    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.oldLayout = old_layout;       // Previous image layout
    barrier.newLayout = new_layout;       // Target image layout
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;                // Target image
    barrier.subresourceRange = subresource_range; // Range of image subresources

    // Set source and destination access masks
    barrier.srcStageMask = src_stage_mask;
    barrier.srcAccessMask = src_access_mask; // Access mask for the previous layout
    barrier.dstStageMask = dst_stage_mask;
    barrier.dstAccessMask = dst_access_mask; // Access mask for the target layout

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;

    // Record the pipeline barrier into the command buffer
    vkCmdPipelineBarrier2(
        command_buffer,  // Command buffer
        &dependencyInfo
    );
}

Vk_Image utils::ImageUtils::create_image(const EngineContext& engine_context, const uint32_t width, const uint32_t height,
                                         const VkFormat format, const VkImageUsageFlags usage_flags, const VmaAllocationCreateInfo& alloc_info, bool create_view,
                                         const bool create_sampler, const VkFilter filter, VkImageAspectFlags aspect_flags)
{
    Vk_Image result_image;
    result_image.format = format;

    auto device_manager = engine_context.device_manager.get();

    // Set up image extent
    VkExtent3D image_extent;
    image_extent.width = width;
    image_extent.height = height;
    image_extent.depth = 1;

    // Create image info
    VkImageCreateInfo img_info = image_create_info(format, usage_flags, image_extent);

    // Use the provided allocation info
    VkResult result = vmaCreateImage(
        device_manager->get_allocator(),
        &img_info,
        &alloc_info,
        &result_image.image,
        &result_image.allocation,
        &result_image.allocation_info
    );

    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create image: " << width << "x" << height
                  << ", format: " << format << std::endl;
        return {};
    }

    // Optionally create image view
    if (create_view)
    {
        create_image_view(engine_context.dispatch_table, result_image, format, aspect_flags);
    }

    // Optionally create a sampler
    if (create_sampler)
    {
        create_image_sampler(engine_context.dispatch_table, result_image, filter);
    }

    std::cout << "Created image: " << width << "x" << height
              << " (" << (create_view ? "with view" : "no view")
              << ", " << (create_sampler ? "with sampler" : "no sampler") << ")" << std::endl;

    return result_image;
}
