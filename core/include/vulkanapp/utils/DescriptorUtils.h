#pragma once
#include <vector>
#include <vma/vk_mem_alloc.h>

#include <VkBootstrapDispatch.h>
#include "structs/Vk_Image.h"

struct GPU_SceneBuffer;
struct GPU_Buffer;
struct Vk_SceneData;

namespace utils
{
    class DescriptorUtils
    {
    public:
        static void setup_texture_descriptors(const vkb::DispatchTable& disp,
                                            const std::vector<Vk_Image>& textures,
                                            VkDescriptorSetLayout& outDescriptorSetLayout,
                                            VkDescriptorSet& outDescriptorSet,
                                            VkDescriptorPool& descriptor_pool);
        
        static VkDescriptorPoolSize descriptor_pool_size(
                VkDescriptorType type,
                uint32_t descriptor_count)
        {
            VkDescriptorPoolSize descriptor_pool_size{};
            descriptor_pool_size.type = type;
            descriptor_pool_size.descriptorCount = descriptor_count;
            return descriptor_pool_size;
        }

        static VkDescriptorSetLayoutBinding descriptor_set_layout_binding(
                VkDescriptorType type,
                VkShaderStageFlags stage_flags,
                uint32_t binding,
                uint32_t descriptor_count = 1)
        {
            VkDescriptorSetLayoutBinding set_layout_binding{};
            set_layout_binding.descriptorType = type;
            set_layout_binding.stageFlags = stage_flags;
            set_layout_binding.binding = binding;
            set_layout_binding.descriptorCount = descriptor_count;
            return set_layout_binding;
        }

        static VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(
                const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        {
            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.pBindings = bindings.data();
            descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            return descriptor_set_layout_create_info;
        }

        static VkDescriptorSetAllocateInfo descriptor_set_allocate_info(
                VkDescriptorPool descriptor_pool,
                const VkDescriptorSetLayout* p_set_layouts,
                uint32_t descriptor_set_count)
        {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = descriptor_pool;
            descriptor_set_allocate_info.pSetLayouts = p_set_layouts;
            descriptor_set_allocate_info.descriptorSetCount = descriptor_set_count;
            return descriptor_set_allocate_info;
        }

        static VkWriteDescriptorSet write_descriptor_set(
                VkDescriptorSet dst_set,
                VkDescriptorType type,
                uint32_t binding,
                VkDescriptorBufferInfo* buffer_info,
                uint32_t descriptor_count = 1)
        {
            VkWriteDescriptorSet write_descriptor_set{};
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.dstSet = dst_set;
            write_descriptor_set.descriptorType = type;
            write_descriptor_set.dstBinding = binding;
            write_descriptor_set.pBufferInfo = buffer_info;
            write_descriptor_set.descriptorCount = descriptor_count;
            return write_descriptor_set;
        }

        static VkDescriptorPoolCreateInfo descriptor_pool_create_info(
                const std::vector<VkDescriptorPoolSize>& pool_sizes,
                uint32_t max_sets)
        {
            VkDescriptorPoolCreateInfo descriptor_pool_info{};
            descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            descriptor_pool_info.pPoolSizes = pool_sizes.data();
            descriptor_pool_info.maxSets = max_sets;
            return descriptor_pool_info;
        }

        static VkPipelineLayoutCreateInfo pipeline_layout_create_info(
                const VkDescriptorSetLayout* p_set_layouts,
                uint32_t set_layout_count,
                const VkPushConstantRange* p_push_constant_ranges = nullptr,
                uint32_t push_constant_range_count = 0)
        {
            VkPipelineLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.setLayoutCount = set_layout_count;
            info.pSetLayouts = p_set_layouts;
            info.pushConstantRangeCount = push_constant_range_count;
            info.pPushConstantRanges = p_push_constant_ranges;
            return info;
        }
    };
};
