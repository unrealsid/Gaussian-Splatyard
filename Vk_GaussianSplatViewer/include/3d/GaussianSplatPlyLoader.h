#pragma once

#include <vector>
#include <string>
#include <glm/vec4.hpp>

#include "structs/geometry/GaussianSurface.h"

namespace camera
{
    class FirstPersonCamera;
}

namespace splat_loader
{
    class GaussianSplatPlyLoader
    {
    public:
        bool load(const std::string& file_path);

        [[nodiscard]] const std::vector<glm::vec4>& get_positions() const { return positions; }
        [[nodiscard]] const std::vector<glm::vec4>& get_scales() const { return scales; }
        [[nodiscard]] const std::vector<float>& get_colors_sh() const { return colors; }
        [[nodiscard]] const std::vector<std::vector<float>>& get_remaining_colors() const { return remaining_colors; }
        [[nodiscard]] const std::vector<glm::vec4>& get_quaternions() const { return quaternions; }
        [[nodiscard]] const std::vector<float>& get_alphas() const { return alphas; }

        [[nodiscard]] std::vector<uint32_t> sort_gaussians(const camera::FirstPersonCamera& camera) const;

        [[nodiscard]] size_t get_count() const { return count; }

    private:
        static constexpr uint32_t sh_dc_count   = 3;
        static constexpr uint32_t sh_rest_count = 45;
        static constexpr uint32_t sh_stride     = sh_dc_count + sh_rest_count;

        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> scales;
        std::vector<float> colors;
        std::vector<std::vector<float>> remaining_colors;
        std::vector<glm::vec4> quaternions;
        std::vector<float> alphas;

        uint32_t count = 0;
    };
}
