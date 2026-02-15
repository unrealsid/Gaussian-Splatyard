#include "3d/ModelUtils.h"

#include <complex.h>
#include <iostream>
#include <random>
#include <vector>
#include "structs/geometry/Vertex2D.h"
#include "3d/GaussianSplatPlyLoader.h"
#include "structs/scene/CameraData.h"
#include "structs/scene/CameraData.h"

namespace entity_3d
{
    std::vector<Vertex2D> ModelUtils::load_triangle_model()
    {
        return std::vector<Vertex2D>
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
    }

    std::vector<glm::vec4> ModelUtils::load_cube()
    {
        std::vector<glm::vec4> cube = {
            // Back
            {-1.f, -1.f, -1.f, 1.f}, { 1.f, -1.f, -1.f, 1.f}, { 1.f,  1.f, -1.f, 1.f},
            { 1.f,  1.f, -1.f, 1.f}, {-1.f,  1.f, -1.f, 1.f}, {-1.f, -1.f, -1.f, 1.f},

            // Front
            {-1.f, -1.f,  1.f, 1.f}, { 1.f, -1.f,  1.f, 1.f}, { 1.f,  1.f,  1.f, 1.f},
            { 1.f,  1.f,  1.f, 1.f}, {-1.f,  1.f,  1.f, 1.f}, {-1.f, -1.f,  1.f, 1.f},

            // Bottom
            {-1.f, -1.f, -1.f, 1.f}, { 1.f, -1.f, -1.f, 1.f}, { 1.f, -1.f,  1.f, 1.f},
            { 1.f, -1.f,  1.f, 1.f}, {-1.f, -1.f,  1.f, 1.f}, {-1.f, -1.f, -1.f, 1.f},

            // Top
            {-1.f,  1.f, -1.f, 1.f}, { 1.f,  1.f, -1.f, 1.f}, { 1.f,  1.f,  1.f, 1.f},
            { 1.f,  1.f,  1.f, 1.f}, {-1.f,  1.f,  1.f, 1.f}, {-1.f,  1.f, -1.f, 1.f},

            // Left
            {-1.f, -1.f, -1.f, 1.f}, {-1.f,  1.f, -1.f, 1.f}, {-1.f,  1.f,  1.f, 1.f},
            {-1.f,  1.f,  1.f, 1.f}, {-1.f, -1.f,  1.f, 1.f}, {-1.f, -1.f, -1.f, 1.f},

            // Right
            { 1.f, -1.f, -1.f, 1.f}, { 1.f,  1.f, -1.f, 1.f}, { 1.f,  1.f,  1.f, 1.f},
            { 1.f,  1.f,  1.f, 1.f}, { 1.f, -1.f,  1.f, 1.f}, { 1.f, -1.f, -1.f, 1.f},
        };

        return cube;
    }

    std::vector<glm::vec2> ModelUtils::load_quad()
    {
        std::vector<glm::vec2> quad =
        {
            {-1.0,  1.0},
            {1.0,  1.0},
            {1.0, -1.0},

            {-1.0,  1.0},
            {1.0, -1.0},
            {-1.0, -1.0}
        };

        return quad;
    }

    std::vector<glm::vec4> ModelUtils::load_tetrahedron()
    {
        // Regular tetrahedron vertices
        constexpr glm::vec4 v0 = {  1.f,  1.f,  1.f, 1.f };
        constexpr glm::vec4 v1 = { -1.f, -1.f,  1.f, 1.f };
        constexpr glm::vec4 v2 = { -1.f,  1.f, -1.f, 1.f };
        constexpr glm::vec4 v3 = {  1.f, -1.f, -1.f, 1.f };

        std::vector tetrahedron =
        {
            // Face 0
            v0, v1, v2,

            // Face 1
            v0, v3, v1,

            // Face 2
            v0, v2, v3,

            // Face 3 (base)
            v1, v3, v2,
        };

        return tetrahedron;
    }


    std::vector<glm::vec4> ModelUtils::generate_vertex_colors(unsigned int vertex_count, bool randomize, const glm::vec4& single_color)
    {
        //constexpr uint32_t vertex_count = 12; // 4 faces * 3 vertices

        std::vector<glm::vec4> colors;
        colors.reserve(vertex_count);

        if (!randomize)
        {
            colors.assign(vertex_count, single_color);
            return colors;
        }

        // Random per-vertex color
        std::mt19937 rng{ std::random_device{}() };
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (uint32_t i = 0; i < vertex_count; ++i)
        {
            colors.emplace_back(
                dist(rng),
                dist(rng),
                dist(rng),
                1.0f
            );
        }

        return colors;
    }

    splat_loader::GaussianSplatPlyLoader ModelUtils::load_placeholder_gaussian_model()
    {
        splat_loader::GaussianSplatPlyLoader ply;

        return ply;
    }

    splat_loader::GaussianSplatPlyLoader ModelUtils::load_gaussian_surfaces(const std::string& file_path)
    {
        splat_loader::GaussianSplatPlyLoader ply;

        if (!ply.load(file_path))
        {
            std::cerr << "Failed to load PLY\n";
            return {};
        }

        return ply;
    }
}
