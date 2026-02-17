#pragma once

#include "Material.h"
#include <memory.h>
#include <string>

struct EngineContext;

namespace material
{
    class MaterialUtils
    {
    public:

        MaterialUtils(EngineContext& engine_context) : engine_context(engine_context){}

        template<typename P>
        [[nodiscard]] std::shared_ptr<Material> create_material(const std::string& name, const std::string& vertex_shader_path, const std::string& fragment_shader_path,
                                                                const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;

        template<typename P>
        [[nodiscard]] std::shared_ptr<Material> create_compute_material(const std::string& name, const std::string& compute_shader_path,
                                                                        const VkDescriptorSetLayout* descriptor_layout, uint32_t descriptor_set_count) const;

    private:
        EngineContext& engine_context;
    };
}
