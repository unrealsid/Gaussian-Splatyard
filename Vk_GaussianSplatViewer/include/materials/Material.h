#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>
#include "materials/ShaderObject.h"

struct EngineContext;

namespace vkb
{
    struct DispatchTable;
}

namespace material
{
    class Material
    {

    public:
        Material(std::string material_name, EngineContext& engine_context);

        //load the shader code
        void init();

        void add_shader_object(std::unique_ptr<ShaderObject> shader_object);
        void add_pipeline_layout(VkPipelineLayout pipeline_layout);
        void add_descriptor_set(VkDescriptorSet descriptor_set);

        [[nodiscard]] VkPipelineLayout get_pipeline_layout() const { return pipeline_layout; }
        [[nodiscard]] ShaderObject* get_shader_object() const { return shader_object.get(); }
        VkDescriptorSet& get_descriptor_set()  { return descriptor_set; }
        [[nodiscard]] std::string get_material_name() const { return material_name; }

        void cleanup();

    private: 
        std::unique_ptr<ShaderObject> shader_object;
        VkDescriptorSet descriptor_set;
        VkPipelineLayout pipeline_layout;

        EngineContext& engine_context;

        std::string material_name;

        std::string vertex_shader_path;
        std::string fragment_shader_path;
    };
}