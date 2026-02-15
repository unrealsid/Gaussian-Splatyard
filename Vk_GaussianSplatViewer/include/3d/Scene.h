#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>
#include <glm/ext/matrix_transform.hpp>

#include "config/Config.inl"
#include "renderer/GPU_BufferContainer.h"
#include "structs/EngineContext.h"
#include "structs/Renderable.h"
#include "vulkanapp/utils/MemoryUtils.h"

struct EngineContext;

namespace entity_3d
{
    class Scene
    {
    public:
        explicit Scene(EngineContext& engine_context) : last_index(0), engine_context(engine_context){ }

        //Call to populate the scene with objects
        void populate_scene();

    private:
        //Adds a new renderable object to the scene given a vertex or index buffer
        template<typename V>
        void add_new_renderable(const std::string& object_name,
                                const std::vector<V>& vertex_buffer,
                                const std::vector<glm::vec4>& vertex_colors,
                                const glm::vec3& position,
                                const glm::vec3& scale,
                                const float angle,
                                const glm::vec3& axis,
                                const uint32_t material_id)
        {
            //make sure we are under the object count
            assert(last_index < max_object_count);

            if (name_to_id.contains(object_name))
            {
                return;
            }

            Renderable renderable;
            core::rendering::GPU_BufferContainer* gpu_buffer_container = engine_context.buffer_container.get();

            //Allocate vertex buffer
            gpu_buffer_container->allocate_named_buffer(object_name + "_vertex_buffer", vertex_buffer, BufferAllocationType::VertexAllocationWithStaging);

            if (const auto buffer = gpu_buffer_container->get_buffer(object_name + "_vertex_buffer"))
            {
                renderable.vertex_buffer = *buffer;
                renderable.vertex_count = vertex_buffer.size();
            }

            renderable.object_index = last_index++;
            renderable.material_index = material_id;

            //add an entry for this object
            if (auto model_matrix_buffer = gpu_buffer_container->get_buffer("model_matrix_buffer"))
            {
                auto model_matrix = glm::mat4(1.0f);
                auto translate = glm::translate(glm::mat4(1.0f), position);
                auto rotate_euler = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
                auto new_scale = glm::scale(glm::mat4(1.0f), scale);

                model_matrix = translate * rotate_euler * new_scale;

                //The object id is the offset in the matrix array
                auto offset = renderable.object_index * sizeof(glm::mat4);
                utils::MemoryUtils::map_persistent_data(engine_context.device_manager->get_allocator(),
                                                        model_matrix_buffer->allocation,
                                                        model_matrix_buffer->allocation_info, &model_matrix, sizeof(glm::mat4), offset);

                renderable.model_matrix_buffer = *model_matrix_buffer;
            }

            //Create color buffer and assign it
            gpu_buffer_container->allocate_named_buffer(object_name + "_color_buffer", vertex_colors);

            if (auto color_buffer = gpu_buffer_container->get_buffer(object_name + "_color_buffer"))
            {
                renderable.vertex_colors_buffer = *color_buffer;
            }

            name_to_id[object_name] = renderable.object_index;

            engine_context.renderer->get_renderables().push_back(renderable);
        }

        //Last object index used
        uint32_t last_index;

        //Stores names to object id
        std::unordered_map<std::string, uint32_t> name_to_id;

        EngineContext& engine_context;
    };
}
