#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "enums/BufferAllocationType.h"
#include "structs/GPU_Buffer.h"

struct EngineContext;

namespace camera
{
    class FirstPersonCamera;
}

namespace core::rendering
{
    class GPU_BufferContainer
    {
    public:
        GPU_BufferContainer(EngineContext& engine_context);

        //Camera Buffer
        GPU_Buffer camera_data_buffer;

        //Stores params for rendering a splats
        GPU_Buffer splat_render_params_buffer;

        //A mapped buffer that stores model matrices for different renderables
        GPU_Buffer model_buffer;

        //How many surfaces has the uploader extracted?
        uint32_t gaussian_count = 0;

        void allocate_camera_buffer(const camera::FirstPersonCamera& first_person_camera);

        //Allocates a large buffer for storing all model matrices. These are offset by Object ID
        void allocate_model_matrices_buffer();

        void allocate_splat_render_params_buffer();

        void map_splat_indices(const std::vector<uint32_t>& indices);

        //Allocates a named buffer
        template<typename N>
        void allocate_named_buffer(const std::string& buffer_name, const std::vector<N>& data, BufferAllocationType allocation_type = BufferAllocationType::VertexAllocationWithStaging);

        GPU_Buffer* get_buffer(const std::string& buffer_name);

        //Gets the GPU buffer for a particular object.
        //We use offsets to decide where the model matrix lives in memory
        VkDeviceAddress get_model_matrix_buffer_offset_address(uint64_t offset);

        void destroy_buffers();

    private:
        EngineContext& engine_context;

        std::unordered_map<std::string, GPU_Buffer> buffers;

        void set_buffer(const std::string& buffer_name, const GPU_Buffer& buffer);
    };
}
