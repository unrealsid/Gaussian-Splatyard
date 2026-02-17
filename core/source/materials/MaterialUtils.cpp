#include "materials/MaterialUtils.h"
#include "materials/Material.h"
#include "structs/scene/PushConstantBlock.h"
#include "vulkanapp/utils/DescriptorUtils.h"
#include "vulkanapp/utils/FileUtils.h"
#include "structs/EngineContext.h"

namespace material
{
    template<typename P>
    std::shared_ptr<Material> MaterialUtils::create_material(const std::string& name, const std::string& vertex_shader_path, const std::string& fragment_shader_path,
                                                             const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const
    {
        //Setup push constants
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(P);

        size_t shaderCodeSizes[2]{};
        char* shaderCodes[2]{};

        utils::FileUtils::load_shader(vertex_shader_path, shaderCodes[0], shaderCodeSizes[0]);
        utils::FileUtils::load_shader(fragment_shader_path, shaderCodes[1], shaderCodeSizes[1]);

        auto shader_object = std::make_unique<ShaderObject>();
        shader_object->create_shaders(engine_context.dispatch_table, shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1],
                                        descriptor_layout, descriptor_set_count,
                                        &push_constant_range, 1);

        VkPipelineLayout pipeline_layout;

        //Create the pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_info = utils::DescriptorUtils::pipeline_layout_create_info(descriptor_layout,  descriptor_set_count, &push_constant_range, 1);
        engine_context.dispatch_table.createPipelineLayout(&pipeline_layout_info, VK_NULL_HANDLE, &pipeline_layout);

        //Create material
        auto material = std::make_shared<Material>(name, engine_context);
        material->add_shader_object(std::move(shader_object));
        material->add_pipeline_layout(pipeline_layout);

        return material;
    }

    template std::shared_ptr<Material> MaterialUtils::create_material<PushConstantBlock>(const std::string& name, const std::string& vertex_shader_path, const std::string& fragment_shader_path,
                                                             const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;

    template<typename P>
    std::shared_ptr<Material> MaterialUtils::create_compute_material(const std::string& name, const std::string& compute_shader_path,
                                                                    const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const
    {
        //Setup push constants
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(P);

        size_t shaderCodeSize{};
        char* shaderCode{};

        utils::FileUtils::load_shader(compute_shader_path, shaderCode, shaderCodeSize);

        auto shader_object = std::make_unique<ShaderObject>();
        shader_object->create_compute_shader(engine_context.dispatch_table, shaderCode, shaderCodeSize,
            descriptor_layout, descriptor_set_count,
            &push_constant_range, 1);

        VkPipelineLayout pipeline_layout;

        //Create the pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_info = utils::DescriptorUtils::pipeline_layout_create_info(descriptor_layout,  descriptor_set_count, &push_constant_range, 1);
        engine_context.dispatch_table.createPipelineLayout(&pipeline_layout_info, VK_NULL_HANDLE, &pipeline_layout);

        //Create material
        auto material = std::make_shared<Material>(name, engine_context);
        material->add_shader_object(std::move(shader_object));
        material->add_pipeline_layout(pipeline_layout);

        return material;
    }

    //TODO: Remove this ;_;
    template std::shared_ptr<Material> MaterialUtils::create_compute_material<PushConstantsComputeCulling>(const std::string& name, const std::string& compute_shader_path, const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;
    template std::shared_ptr<Material> MaterialUtils::create_compute_material<PushConstantsComputeHistograms>(const std::string& name, const std::string& compute_shader_path, const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;
    template std::shared_ptr<Material> MaterialUtils::create_compute_material<PushConstantsRadixSort>(const std::string& name, const std::string& compute_shader_path, const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;
}
