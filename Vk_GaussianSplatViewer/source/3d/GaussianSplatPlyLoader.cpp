#include "3d/GaussianSplatPlyLoader.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <numeric>
#include <tinyply.h>
#include <glm/fwd.hpp>
#include <glm/detail/func_geometric.inl>
#include "camera/FirstPersonCamera.h"

using namespace tinyply;

namespace splat_loader
{
    namespace
    {
        template<typename T>
        void copy_property(
            const std::shared_ptr<PlyData>& data,
            T* dst,
            size_t count)
        {
            std::memcpy(dst, data->buffer.get(), sizeof(T) * count);
        }

        float sigmoid(float x)
        {
            return 1.0f / (1.0f + std::exp(-x));
        }
    }

    bool GaussianSplatPlyLoader::load(const std::string& file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file)
        {
            std::cerr << "Failed to open PLY file: " << file_path << std::endl;
            return false;
        }

        PlyFile ply_file;
        ply_file.parse_header(file);

        // --- Request properties ---
        auto x  = ply_file.request_properties_from_element("vertex", { "x" });
        auto y  = ply_file.request_properties_from_element("vertex", { "y" });
        auto z  = ply_file.request_properties_from_element("vertex", { "z" });

        auto nx = ply_file.request_properties_from_element("vertex", { "nx" });
        auto ny = ply_file.request_properties_from_element("vertex", { "ny" });
        auto nz = ply_file.request_properties_from_element("vertex", { "nz" });

        auto opacity = ply_file.request_properties_from_element("vertex", { "opacity" });

        auto scale_0 = ply_file.request_properties_from_element("vertex", { "scale_0" });
        auto scale_1 = ply_file.request_properties_from_element("vertex", { "scale_1" });
        auto scale_2 = ply_file.request_properties_from_element("vertex", { "scale_2" });

        auto rot_0 = ply_file.request_properties_from_element("vertex", { "rot_0" });
        auto rot_1 = ply_file.request_properties_from_element("vertex", { "rot_1" });
        auto rot_2 = ply_file.request_properties_from_element("vertex", { "rot_2" });
        auto rot_3 = ply_file.request_properties_from_element("vertex", { "rot_3" });

        std::shared_ptr<PlyData> f_dc[3];
        std::shared_ptr<PlyData> f_rest[45];

        for (int i = 0; i < 3; ++i)
            f_dc[i] = ply_file.request_properties_from_element(
                "vertex", { "f_dc_" + std::to_string(i) });

        for (int i = 0; i < 45; ++i)
            f_rest[i] = ply_file.request_properties_from_element(
                "vertex", { "f_rest_" + std::to_string(i) });

        // Read data
        ply_file.read(file);

        const size_t vertex_count = x->count;
        count = vertex_count;

        // --- Allocate temporary SoA buffers ---
        std::vector<float> xs(vertex_count), ys(vertex_count), zs(vertex_count);
        std::vector<float> nxs(vertex_count), nys(vertex_count), nzs(vertex_count);
        std::vector<float> opacity_arr(vertex_count);

        std::vector<float> scale[3];
        std::vector<float> rotation[4];
        std::vector<float> f_dc_arr[3];
        std::vector<float> f_rest_arr[45];

        for (int i = 0; i < 3; ++i)
        {
            scale[i].resize(vertex_count);
            f_dc_arr[i].resize(vertex_count);
        }

        for (auto & i : rotation)
        {
            i.resize(vertex_count);
        }

        for (auto & i : f_rest_arr)
        {
            i.resize(vertex_count);
        }

        copy_property(x,  xs.data(), vertex_count);
        copy_property(y,  ys.data(), vertex_count);
        copy_property(z,  zs.data(), vertex_count);

        copy_property(nx, nxs.data(), vertex_count);
        copy_property(ny, nys.data(), vertex_count);
        copy_property(nz, nzs.data(), vertex_count);

        copy_property(opacity, opacity_arr.data(), vertex_count);

        copy_property(scale_0, scale[0].data(), vertex_count);
        copy_property(scale_1, scale[1].data(), vertex_count);
        copy_property(scale_2, scale[2].data(), vertex_count);

        copy_property(rot_0, rotation[0].data(), vertex_count);
        copy_property(rot_1, rotation[1].data(), vertex_count);
        copy_property(rot_2, rotation[2].data(), vertex_count);
        copy_property(rot_3, rotation[3].data(), vertex_count);

        for (int i = 0; i < 3; ++i)
            copy_property(f_dc[i], f_dc_arr[i].data(), vertex_count);

        for (int i = 0; i < 45; ++i)
            copy_property(f_rest[i], f_rest_arr[i].data(), vertex_count);

        positions.resize(vertex_count);
        scales.resize(vertex_count);
        colors.resize(vertex_count * sh_stride);
        quaternions.resize(vertex_count);
        alphas.resize(vertex_count);

        for (size_t i = 0; i < vertex_count; ++i)
        {
            positions[i] = glm::vec4(xs[i], ys[i], zs[i], 1.0f);

            scales[i] = glm::vec4(
                std::exp(scale[0][i]),
                std::exp(scale[1][i]),
                std::exp(scale[2][i]),
                1.0f
            );

            const size_t base = i * sh_stride;

            // f_dc
            colors[base + 0] = f_dc_arr[0][i];
            colors[base + 1] = f_dc_arr[1][i];
            colors[base + 2] = f_dc_arr[2][i];

            // f_rest
            for (uint32_t j = 0; j < sh_rest_count; ++j)
            {
                colors[base + sh_dc_count + j] = f_rest_arr[j][i];
            }

            glm::vec4 quat(rotation[0][i], rotation[1][i], rotation[2][i], rotation[3][i]);
            quaternions[i] = glm::normalize(quat);

            alphas[i] = sigmoid(opacity_arr[i]);
        }

        return true;
    }

    std::vector<uint32_t> GaussianSplatPlyLoader::sort_gaussians(const camera::FirstPersonCamera& camera) const
    {
        const auto count = static_cast<uint32_t>(positions.size());
        std::vector<uint32_t> indices(count);
        std::iota(indices.begin(), indices.end(), 0);

        glm::mat4 view_mat = camera.get_view_matrix();

        std::vector<float> depths(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            // The Python template:
            // xyz_view = view_mat[None, :3, :3] @ xyz[..., None] + view_mat[None, :3, 3, None]
            // depth = xyz_view[:, 2, 0]

            // In GLM/C++, transforming to view space:
            // xyz_view = view_mat * glm::vec4(xyz, 1.0)
            // depth is the Z component (index 2)

            glm::vec4 pos_view = view_mat * glm::vec4(glm::vec3(positions[i]), 1.0f);
            depths[i] = pos_view.z;
        }

        std::ranges::sort(indices, [&](uint32_t a, uint32_t b)
        {
            return depths[a] < depths[b];
        });

        return indices;
    }
}