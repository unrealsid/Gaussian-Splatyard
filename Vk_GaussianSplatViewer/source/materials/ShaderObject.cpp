#include "materials/ShaderObject.h"

#include "../../include/structs/geometry/Vertex.h"
#include "vulkanapp/SwapchainManager.h"
#include <iostream>
#include <utility>

material::ShaderObject::Shader::Shader(VkShaderStageFlagBits        stage_,
                             VkShaderStageFlags           next_stage_,
                             std::string                  shader_name_,
                             char*						  glsl_source,
                             size_t						  spirv_size,
                             const VkDescriptorSetLayout *pSetLayouts,
                             uint32_t                     setLayoutCount,
                             const VkPushConstantRange   *pPushConstantRange,
                             const  uint32_t			  pPushConstantCount)
{
    stage       = stage_;
    shader_name = std::move(shader_name_);
    next_stage  = next_stage_;
    spirv       = glsl_source;

    // Fill out the shader create info struct
    vk_shader_create_info.sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
    vk_shader_create_info.pNext                  = nullptr;
    vk_shader_create_info.flags                  = 0;
    vk_shader_create_info.stage                  = stage;
    vk_shader_create_info.nextStage              = next_stage;
    vk_shader_create_info.codeType               = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vk_shader_create_info.codeSize               = spirv_size;
    vk_shader_create_info.pCode                  = spirv;
    vk_shader_create_info.pName                  = "main";
    vk_shader_create_info.setLayoutCount         = setLayoutCount;
    vk_shader_create_info.pSetLayouts            = pSetLayouts;
    vk_shader_create_info.pushConstantRangeCount = pPushConstantCount;
    vk_shader_create_info.pPushConstantRanges    = pPushConstantRange;
    vk_shader_create_info.pSpecializationInfo    = nullptr;
}


void material::ShaderObject::Shader::destroy(const vkb::DispatchTable& disp)
{
    if (shader != VK_NULL_HANDLE)
    {
        disp.destroyShaderEXT(shader, nullptr);
        shader = VK_NULL_HANDLE;
    }
}

void material::ShaderObject::build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag)
{
    VkShaderCreateInfoEXT shader_create_infos[2];

    if (vert == nullptr || frag == nullptr)
    {
        std::cerr << ("build_linked_shaders failed with null vertex or fragment shader\n");
    }

    shader_create_infos[0] = vert->get_create_info();
    shader_create_infos[1] = frag->get_create_info();

    for (auto &shader_create : shader_create_infos)
    {
        shader_create.flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    }

    VkShaderEXT shaderEXTs[2];

    // Create the shader objects
    VkResult result = disp.createShadersEXT(
                                         2,
                                         shader_create_infos,
                                         nullptr,
                                         shaderEXTs);

    if (result != VK_SUCCESS)
    {
        std::cerr << ("vkCreateShadersEXT failed\n");
    }

    vert->set_shader(shaderEXTs[0]);
    frag->set_shader(shaderEXTs[1]);
}

void material::ShaderObject::create_shaders(const vkb::DispatchTable& disp, char* vertexShader, size_t vertShaderSize, char* fragmentShader, size_t fragShaderSize,
	const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount,
	const VkPushConstantRange* pPushConstantRange, uint32_t pPushConstantCount)
{
	vert_shader = std::make_unique<Shader>(VK_SHADER_STAGE_VERTEX_BIT,
	                                      VK_SHADER_STAGE_FRAGMENT_BIT,
	                                      "VertexShader", vertexShader,
	                                      vertShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);
                                        
    frag_shader = std::make_unique<Shader>(VK_SHADER_STAGE_FRAGMENT_BIT,
                                    0,
                                    "FragmentShader",
                                    fragmentShader,
                                    fragShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);

    build_linked_shaders(disp, vert_shader.get(), frag_shader.get());
}

void material::ShaderObject::create_compute_shader(const vkb::DispatchTable& disp, char* computeShader, size_t computeShaderSize,
    const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount,
    const VkPushConstantRange* pPushConstantRange, uint32_t pPushConstantCount)
{
    compute_shader = std::make_unique<Shader>(VK_SHADER_STAGE_COMPUTE_BIT,
                                           0,
                                           "ComputeShader",
                                           computeShader,
                                           computeShaderSize,
                                           pSetLayouts,
                                           setLayoutCount,
                                           pPushConstantRange,
                                           pPushConstantCount);

    VkShaderCreateInfoEXT create_info = compute_shader->get_create_info();
    VkShaderEXT shaderEXT;
    disp.createShadersEXT(1, &create_info, nullptr, &shaderEXT);
    compute_shader->set_shader(shaderEXT);
}

void material::ShaderObject::destroy_shaders(const vkb::DispatchTable& disp)
{
    if (vert_shader)
    {
        vert_shader->destroy(disp);
    }
    if (frag_shader)
    {
        frag_shader->destroy(disp);
    }
    if (compute_shader)
    {
        compute_shader->destroy(disp);
    }
}

void material::ShaderObject::bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader* shader)
{
    disp.cmdBindShadersEXT(cmd_buffer, 1, shader->get_stage(), shader->get_shader());
}

void material::ShaderObject::bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const
{
    if (compute_shader)
    {
        bind_shader(disp, cmd_buffer, compute_shader.get());
    }
    else
    {
        bind_shader(disp, cmd_buffer, vert_shader.get());
        bind_shader(disp, cmd_buffer, frag_shader.get());
    }
}

void material::ShaderObject::set_initial_state(vkb::DispatchTable& disp, VkExtent2D viewport_extent,
    VkCommandBuffer cmd_buffer, const std::vector<VkVertexInputBindingDescription2EXT>& vertex_input_binding,
    const std::vector<VkVertexInputAttributeDescription2EXT>& input_attribute_description, VkExtent2D scissor_extents,
    VkOffset2D scissor_offset, const std::vector<VkColorComponentFlags>& color_component_flags,
    const std::vector<VkBool32>& color_blend_enables)
{
    {
		{
			// Set viewport and scissor
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(viewport_extent.width);
			viewport.height = static_cast<float>(viewport_extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = scissor_offset;
			scissor.extent = scissor_extents;

			disp.cmdSetViewportWithCountEXT(cmd_buffer, 1, &viewport);
			disp.cmdSetScissorWithCountEXT(cmd_buffer, 1, &scissor);

			disp.cmdSetScissor(cmd_buffer, 0, 1, &scissor);

			disp.cmdSetCullModeEXT(cmd_buffer, VK_CULL_MODE_NONE);
			disp.cmdSetFrontFaceEXT(cmd_buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
			disp.cmdSetDepthTestEnableEXT(cmd_buffer, VK_TRUE);
			disp.cmdSetDepthWriteEnableEXT(cmd_buffer, VK_TRUE);
			disp.cmdSetDepthCompareOpEXT(cmd_buffer, VK_COMPARE_OP_LESS);
			disp.cmdSetPrimitiveTopologyEXT(cmd_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN); //VK_PRIMITIVE_TOPOLOGY_POINT_LIST for point
			disp.cmdSetRasterizerDiscardEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetPolygonModeEXT(cmd_buffer, VK_POLYGON_MODE_FILL); //VK_POLYGON_MODE_POINT for point
			disp.cmdSetRasterizationSamplesEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT);
			disp.cmdSetAlphaToCoverageEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetDepthBiasEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetStencilTestEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetPrimitiveRestartEnableEXT(cmd_buffer, VK_FALSE);

			const VkSampleMask sample_mask = 0xFF;
			disp.cmdSetSampleMaskEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT, &sample_mask);

			// Disable color blending
			//VkBool32 color_blend_enables= VK_FALSE;
			disp.cmdSetColorBlendEnableEXT(cmd_buffer, 0, color_blend_enables.size(), color_blend_enables.data());

			// Use RGBA color write mask
			//VkColorComponentFlags color_component_flags = 0xF;
			disp.cmdSetColorWriteMaskEXT(cmd_buffer, 0, color_component_flags.size(), color_component_flags.data());
		}

		//Vertex input
		{
			// Get the vertex binding and attribute descriptions
			// auto bindingDescription = vertex_input_binding;  //Vertex::get_binding_description();
			// auto attributeDescriptions = input_attribute_description; //Vertex::get_attribute_descriptions();

			// Set the vertex input state using the descriptions
			disp.cmdSetVertexInputEXT
			(
				cmd_buffer,
				vertex_input_binding.size(),                                                          // bindingCount = 1 (we have one vertex buffer binding)
				vertex_input_binding.data(),                                        // pVertexBindingDescriptions
				input_attribute_description.size(),								// attributeCount
				input_attribute_description.data()                                // pVertexAttributeDescriptions
			);
		}
	};
}
