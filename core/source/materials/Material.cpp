#include "materials/Material.h"
#include "materials/ShaderObject.h"
#include "structs/EngineContext.h"
#include "structs/scene/PushConstantBlock.h"
#include "vulkanapp/utils/FileUtils.h"

namespace material
{
    Material::Material(std::string material_name, EngineContext& engine_context) :
        material_name(std::move(material_name)), descriptor_set(nullptr), pipeline_layout(nullptr),
        engine_context(engine_context)
    {
        
    }

    void Material::init()
    {
    }

    void Material::add_shader_object(std::unique_ptr<ShaderObject> shader_object)
    {
        this->shader_object = std::move(shader_object);
    }

    void Material::add_pipeline_layout(VkPipelineLayout pipeline_layout)
    {
        this->pipeline_layout = pipeline_layout;
    }

    void Material::add_descriptor_set(VkDescriptorSet descriptor_set)
    {
        this->descriptor_set = descriptor_set;
    }

    void Material::cleanup()
    {
        if (pipeline_layout != VK_NULL_HANDLE)
        {
            engine_context.dispatch_table.destroyPipelineLayout(pipeline_layout, nullptr);
            pipeline_layout = VK_NULL_HANDLE;
        }

        if (shader_object)
        {
            shader_object->destroy_shaders(engine_context.dispatch_table);
        }
    }
}
