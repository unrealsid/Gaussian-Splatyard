#pragma once
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <array>

#include "VkBootstrapDispatch.h"

struct Init;

namespace material
{
	class ShaderObject
	{
	public:
		class Shader
		{
			VkShaderStageFlagBits stage{};
			VkShaderStageFlags    next_stage{};
			VkShaderEXT           shader      = VK_NULL_HANDLE;
			std::string           shader_name = "shader";
			VkShaderCreateInfoEXT vk_shader_create_info{};
			char* spirv = nullptr;
			size_t spirv_size = 0;

		public:
			Shader(){}
			Shader(VkShaderStageFlagBits        stage,
				   VkShaderStageFlags           next_stage,
				   std::string                  name,
				   char*                        glsl_source,
				   size_t                       spir_size,
				   const VkDescriptorSetLayout *pSetLayouts,
				   uint32_t                     setLayoutCount,
				   const VkPushConstantRange   *pPushConstantRange,
				   const  uint32_t			    pPushConstantCount);

			std::string get_name()
			{
				return shader_name;
			}

			VkShaderCreateInfoEXT get_create_info() const
			{
				return vk_shader_create_info;
			}

			const VkShaderEXT *get_shader() const
			{
				return &shader;
			}

			const VkShaderStageFlagBits *get_stage() const
			{
				return &stage;
			}

			const VkShaderStageFlags *get_next_stage() const
			{
				return &next_stage;
			}

			void set_shader(VkShaderEXT _shader)
			{
				shader = _shader;
			}

			void destroy(const vkb::DispatchTable& disp);
		};

		ShaderObject() = default;
		~ShaderObject() = default;

		void create_shaders(const vkb::DispatchTable& disp,
			char* vertexShader, size_t vertShaderSize,
			char* fragmentShader, size_t fragShaderSize,
			const VkDescriptorSetLayout *pSetLayouts, uint32_t setLayoutCount,
			const VkPushConstantRange *pPushConstantRange, uint32_t pPushConstantCount);

		void create_compute_shader(const vkb::DispatchTable& disp,
			char* computeShader, size_t computeShaderSize,
			const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount,
			const VkPushConstantRange* pPushConstantRange, uint32_t pPushConstantCount);
    
		void destroy_shaders(const vkb::DispatchTable& disp);

		static void bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader *shader);
		void bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const;

		static void set_initial_state(vkb::DispatchTable& disp, VkExtent2D viewport_extent, VkCommandBuffer cmd_buffer,
									  const std::vector<VkVertexInputBindingDescription2EXT>& vertex_input_binding,
									  const std::vector<VkVertexInputAttributeDescription2EXT>& input_attribute_description,
		                              VkExtent2D scissor_extents, VkOffset2D scissor_offset,
		                              const std::vector<VkColorComponentFlags>& color_component_flags,
		                              const std::vector<VkBool32>& color_blend_enables);

	private:
		static void build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag);
    
		std::unique_ptr<Shader> vert_shader;
		std::unique_ptr<Shader> frag_shader;
		std::unique_ptr<Shader> compute_shader;
	};
}
