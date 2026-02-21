#include "renderer/GPU_BufferContainer.h"

#include <ranges>

#include "enums/BufferAllocationType.h"
#include "renderer/RenderPass.h"
#include "structs/EngineContext.h"
#include "structs/scene/CameraData.h"
#include "vulkanapp/utils/MemoryUtils.h"
#include "config/Config.inl"
#include <iostream>

#include "structs/scene/SplatRenderingParams.h"

namespace core::rendering
{
    GPU_BufferContainer::GPU_BufferContainer(EngineContext& engine_context) : engine_context(engine_context)
    {
        std::cout << "init";
    }

    void GPU_BufferContainer::allocate_camera_buffer(const camera::FirstPersonCamera& first_person_camera)
    {
        CameraData ubo{};

        ubo.projection = first_person_camera.get_projection_matrix();
        ubo.view = first_person_camera.get_view_matrix();
        ubo.camera_position = glm::vec4(first_person_camera.get_position(), 1.0);
        ubo.hfovxy_focal = glm::vec4(first_person_camera.get_htanfovxy_focal(window_height), 1.0);

        utils::MemoryUtils::allocate_buffer_with_mapped_access(
            engine_context.dispatch_table,
            engine_context.device_manager->get_allocator(),
            sizeof(CameraData),
            camera_data_buffer
        );

        memcpy(camera_data_buffer.allocation_info.pMappedData, &ubo, sizeof(CameraData));

        vmaFlushAllocation(
            engine_context.device_manager->get_allocator(),
            camera_data_buffer .allocation,
            0,
            VK_WHOLE_SIZE
        );
    }

    void GPU_BufferContainer::allocate_splat_render_params_buffer()
    {
        SplatRenderingParams splat_rendering_params{};

        splat_rendering_params.rendering_params = glm::vec4(0.0f);

        utils::MemoryUtils::allocate_buffer_with_mapped_access(
                   engine_context.dispatch_table,
                   engine_context.device_manager->get_allocator(),
                   sizeof(SplatRenderingParams),
                   splat_render_params_buffer
               );

        memcpy(splat_render_params_buffer.allocation_info.pMappedData, &splat_rendering_params, sizeof(SplatRenderingParams));

        vmaFlushAllocation(
            engine_context.device_manager->get_allocator(),
            splat_render_params_buffer .allocation,
            0,
            VK_WHOLE_SIZE
        );
    }

    void GPU_BufferContainer::map_splat_indices(const std::vector<uint32_t>& indices)
    {
        if (auto buffer = get_buffer("splat_indices_buffer"))
        {
            memcpy(buffer->allocation_info.pMappedData, indices.data(), sizeof(uint32_t) * indices.size());

            vmaFlushAllocation(
                engine_context.device_manager->get_allocator(),
                splat_render_params_buffer .allocation,
                0,
                VK_WHOLE_SIZE
            );
        }
    }

    void GPU_BufferContainer::map_data(const std::string& buffer_name, void* data, size_t size)
    {
        if (auto buffer = get_buffer(buffer_name))
        {
            memcpy(buffer->allocation_info.pMappedData, data, size);

            vmaFlushAllocation(
                engine_context.device_manager->get_allocator(),
                splat_render_params_buffer .allocation,
                0,
                VK_WHOLE_SIZE
            );
        }
    }

    void GPU_BufferContainer::allocate_model_matrices_buffer()
    {
        //Allocate a named mapped buffer that supports max_object count of objects for storing model matrices
        allocate_named_buffer("model_matrix_buffer", std::vector<glm::mat4>(max_object_count), BufferAllocationType::MappedAllocation);
    }

    void GPU_BufferContainer::set_buffer(const std::string& buffer_name, const GPU_Buffer& buffer)
    {
        buffers[buffer_name] = buffer;
    }

    GPU_Buffer* GPU_BufferContainer::get_buffer(const std::string& buffer_name)
    {
        const auto it = buffers.find(buffer_name);
        if (it == buffers.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    VkDeviceAddress GPU_BufferContainer::get_model_matrix_buffer_offset_address(uint64_t offset)
    {
        if (auto buffer = get_buffer("model_matrix_buffer"))
        {
            // Align the offset first
            constexpr size_t mat4Size = sizeof(glm::mat4);
            size_t aligned_offset = offset * mat4Size;

            return buffer->buffer_address + aligned_offset;
        }

        return {};
    }

    void GPU_BufferContainer::destroy_buffers()
    {
        utils::MemoryUtils::destroy_buffer(engine_context.device_manager->get_allocator(), camera_data_buffer);

        for (auto& val : buffers | std::views::values)
        {
            utils::MemoryUtils::destroy_buffer(engine_context.device_manager->get_allocator(), val);
        }
    }

    template <typename N>
    void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<N>& data, BufferAllocationType allocation_type)
    {
        //Wait for the device to be idle to avoid destroying any tasks that are ongoing
        // ReSharper disable once CppExpressionWithoutSideEffects
        engine_context.dispatch_table.deviceWaitIdle();

        auto device_manager = engine_context.device_manager.get();

        //Destroy the old buffer if we have a buffer with the same name
        if (auto buffer = get_buffer(buffer_name))
        {
            vmaDestroyBuffer(device_manager->get_allocator(), buffer->buffer, buffer->allocation);
            *buffer = { VK_NULL_HANDLE, VK_NULL_HANDLE, {}, {} };
            buffers.erase(buffer_name);
        }

        //Recreate it
        GPU_Buffer new_buffer;

        switch (allocation_type)
        {
        case BufferAllocationType::None:
            break;
        case BufferAllocationType::VertexAllocationWithStaging:
            utils::MemoryUtils::create_vertex_buffer_with_staging(engine_context,
                                                                  data,
                                                                  engine_context.renderer->get_render_pass()->get_command_pool(),
                                                                  new_buffer);
            break;

        case BufferAllocationType::MappedAllocation:
            utils::MemoryUtils::allocate_buffer_with_mapped_access(engine_context.dispatch_table,
                                                                   engine_context.device_manager->get_allocator(),
                                                                   data.size() * sizeof(N),
                                                                   new_buffer);
            break;
        }

        set_buffer(buffer_name, new_buffer);
    }

    template void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<glm::vec4>& gaussian_parameters, BufferAllocationType allocation_type);
    template void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<float>& gaussian_parameters, BufferAllocationType allocation_type);
    template void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<glm::vec2>& gaussian_parameters, BufferAllocationType allocation_type);
    template void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<uint32_t>& gaussian_parameters, BufferAllocationType allocation_type);
    template void GPU_BufferContainer::allocate_named_buffer(const std::string& buffer_name, const std::vector<glm::uvec2>& gaussian_parameters, BufferAllocationType allocation_type);

    template <typename N>
    void GPU_BufferContainer::allocate_named_buffer_simple(const std::string& buffer_name, BufferAllocationType allocation_type)
    {
        //Wait for the device to be idle to avoid destroying any tasks that are ongoing
        // ReSharper disable once CppExpressionWithoutSideEffects
        engine_context.dispatch_table.deviceWaitIdle();

        auto device_manager = engine_context.device_manager.get();

        //Destroy the old buffer if we have a buffer with the same name
        if (auto buffer = get_buffer(buffer_name))
        {
            vmaDestroyBuffer(device_manager->get_allocator(), buffer->buffer, buffer->allocation);
            *buffer = { VK_NULL_HANDLE, VK_NULL_HANDLE, {}, {} };
            buffers.erase(buffer_name);
        }

        //Recreate it
        GPU_Buffer new_buffer;

        switch (allocation_type)
        {
        case BufferAllocationType::None:
            break;

        case BufferAllocationType::VertexAllocationWithStaging:
            //@TODO
            break;

        case BufferAllocationType::MappedAllocation:
            utils::MemoryUtils::allocate_buffer_with_mapped_access(engine_context.dispatch_table,
                                                                   engine_context.device_manager->get_allocator(),
                                                                   sizeof(N),
                                                                   new_buffer);
            break;
        }

        set_buffer(buffer_name, new_buffer);
    }

    template void GPU_BufferContainer::allocate_named_buffer_simple<int>(const std::string& buffer_name, BufferAllocationType allocation_type);
}
