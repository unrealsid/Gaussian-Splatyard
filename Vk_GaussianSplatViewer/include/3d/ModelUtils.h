#pragma once
#include <string>

#include "GaussianSplatPlyLoader.h"
#include "structs/geometry/Vertex2D.h"

namespace entity_3d
{
    class ModelUtils
    {
        public:
        static std::vector<Vertex2D> load_triangle_model();

        static std::vector<glm::vec4> load_cube();

        static std::vector<glm::vec2> load_quad();

        static std::vector<glm::vec4> load_tetrahedron();

        //Generate random/fixed colors per vertex
        //12 vertices for a tetrahedron and 36 for a cube
        static std::vector<glm::vec4> generate_vertex_colors(uint32_t vertex_count, bool randomize, const glm::vec4& single_color = { 1.f, 1.f, 1.f, 1.f });

        static splat_loader::GaussianSplatPlyLoader load_placeholder_gaussian_model();

        static splat_loader::GaussianSplatPlyLoader load_gaussian_surfaces(const std::string& file_path);
    };
}
