#version 460

#extension GL_ARB_shading_language_include : require

#include "../../common/gaussian_common.glsl"

#define SH_C0 0.28209479177387814f
#define SH_C1 0.4886025119029199f

#define SH_C2_0 1.0925484305920792f
#define SH_C2_1 -1.0925484305920792f
#define SH_C2_2 0.31539156525252005f
#define SH_C2_3 -1.0925484305920792f
#define SH_C2_4 0.5462742152960396f

#define SH_C3_0 -0.5900435899266435f
#define SH_C3_1 2.890611442640554f
#define SH_C3_2 -0.4570457994644658f
#define SH_C3_3 0.3731763325901154f
#define SH_C3_4 -0.4570457994644658f
#define SH_C3_5 1.445305721320277f
#define SH_C3_6 -0.5900435899266435f

layout(location = 0) in vec2 position;

layout(location = 0) out Interpolants OUT;

mat3 computeCov3D(vec3 scale, vec4 q)  // should be correct
{
    mat3 S = mat3(0.f);
    S[0][0] = scale.x;
    S[1][1] = scale.y;
    S[2][2] = scale.z;
    float r = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    mat3 R = mat3(
    1.f - 2.f * (y * y + z * z), 2.f * (x * y - r * z), 2.f * (x * z + r * y),
    2.f * (x * y + r * z), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - r * x),
    2.f * (x * z - r * y), 2.f * (y * z + r * x), 1.f - 2.f * (x * x + y * y)
    );

    OUT.normal = normalize(R * vec3(0.0, 0.0, 1.0));

    mat3 M = S * R;
    mat3 Sigma = transpose(M) * M;
    return Sigma;
}

vec3 computeCov2D(vec4 mean_view, float focal_x, float focal_y, float tan_fovx, float tan_fovy, mat3 cov3D, mat4 viewmatrix)
{
    vec4 t = mean_view;
    // why need this? Try remove this later
    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    mat3 J = mat3(
    focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
    0.0f, -focal_y / t.z, (focal_y * t.y) / (t.z * t.z),
    0, 0, 0
    );
    mat3 W = transpose(mat3(viewmatrix));
    mat3 T = W * J;

    mat3 cov = transpose(T) * transpose(cov3D) * T;
    // Apply low-pass filter: every Gaussian should be at least
    // one pixel wide/high. Discard 3rd row and column.
    cov[0][0] += 0.3f;
    cov[1][1] += 0.3f;
    return vec3(cov[0][0], cov[0][1], cov[1][1]);
}

void main()
{
    CameraData camera_matrices = pc.camera_data_adddress;
    SplatID splat_ids = pc.splat_ids_address;
    SplatPosition splat_positions = pc.splat_positions_address;
    SplatScale splat_scale = pc.splat_scales_address;
    SplatColor splat_color = pc.splat_colors_address;
    SplatQuat splat_quat = pc.splat_quats_address;
    SplatAlpha splat_alpha = pc.splat_alphas_address;

    int splat_id = splat_ids.gi[gl_InstanceIndex];

    vec4 g_pos = splat_positions.positions[splat_id];
    OUT.position = g_pos.xyz;

    vec4 g_pos_view = camera_matrices.view_matrix * g_pos;
    vec4 g_pos_screen = camera_matrices.projection_matrix * g_pos_view;

    // Perspective divide
    g_pos_screen.xyz = g_pos_screen.xyz / g_pos_screen.w;
    g_pos_screen.w = 1.0;

    // Early culling
    if (any(greaterThan(abs(g_pos_screen.xyz), vec3(1.3))))
    {
        gl_Position = vec4(-100, -100, -100, 1);
        return;
    }

    OUT.depth = -g_pos_view.z;

    vec4 g_rot = splat_quat.quats[splat_id];
    vec3 g_scale = splat_scale.scales[splat_id].xyz;
    OUT.alpha = splat_alpha.alphas[splat_id];

    mat3 cov3d = computeCov3D(g_scale, g_rot);

    vec4 hfovxy_focal = camera_matrices.hfovxy_focal;
    vec2 wh = 2 * hfovxy_focal.xy * hfovxy_focal.z;
    vec3 cov2d = computeCov2D(g_pos_view, hfovxy_focal.z, hfovxy_focal.z, hfovxy_focal.x, hfovxy_focal.y, cov3d, camera_matrices.view_matrix);

    // Invert covariance (EWA algorithm)
    float det = (cov2d.x * cov2d.z - cov2d.y * cov2d.y);
    if (det == 0.0f)
    {
        gl_Position = vec4(0.f, 0.f, 0.f, 0.f);
        return;
    }

    float det_inv = 1.f / det;
    OUT.conic = vec3(cov2d.z * det_inv, -cov2d.y * det_inv, cov2d.x * det_inv);
    vec2 quadwh_scr = vec2(3.f * sqrt(cov2d.x), 3.f * sqrt(cov2d.z));
    vec2 quadwh_ndc = quadwh_scr / wh * 2;

    g_pos_screen.xy = g_pos_screen.xy + position * quadwh_ndc;
    OUT.coordxy = position * quadwh_scr;
    gl_Position = g_pos_screen;

    // --- SH Color Calculation (Standard) ---
    vec3 dir = normalize(g_pos.xyz - camera_matrices.camera_position.xyz);
    OUT.view_direction = -dir;

    int base = splat_id * 48;

    // Degree 0
    vec3 final_color = SH_C0 * vec3(splat_color.colors[base + 0], splat_color.colors[base + 1], splat_color.colors[base + 2]);

    // Degree 1
//    float x = dir.x; float y = dir.y; float z = dir.z;
//    final_color -= SH_C1 * y * vec3(splat_color.colors[base + 3], splat_color.colors[base + 4], splat_color.colors[base + 5]);
//    final_color += SH_C1 * z * vec3(splat_color.colors[base + 6], splat_color.colors[base + 7], splat_color.colors[base + 8]);
//    final_color -= SH_C1 * x * vec3(splat_color.colors[base + 9], splat_color.colors[base + 10], splat_color.colors[base + 11]);
//
//    // Degree 2
//    float xx = x * x, yy = y * y, zz = z * z, xy = x * y, yz = y * z, xz = x * z;
//    final_color += SH_C2_0 * xy * vec3(splat_color.colors[base + 12], splat_color.colors[base + 13], splat_color.colors[base + 14]) +
//    SH_C2_1 * yz * vec3(splat_color.colors[base + 15], splat_color.colors[base + 16], splat_color.colors[base + 17]) +
//    SH_C2_2 * (2.0 * zz - xx - yy) * vec3(splat_color.colors[base + 18], splat_color.colors[base + 19], splat_color.colors[base + 20]) +
//    SH_C2_3 * xz * vec3(splat_color.colors[base + 21], splat_color.colors[base + 22], splat_color.colors[base + 23]) +
//    SH_C2_4 * (xx - yy) * vec3(splat_color.colors[base + 24], splat_color.colors[base + 25], splat_color.colors[base + 26]);
//
//    // Degree 3
//    final_color += SH_C3_0 * y * (3.0 * xx - yy) * vec3(splat_color.colors[base + 27], splat_color.colors[base + 28], splat_color.colors[base + 29]) +
//    SH_C3_1 * xy * z * vec3(splat_color.colors[base + 30], splat_color.colors[base + 31], splat_color.colors[base + 32]) +
//    SH_C3_2 * y * (4.0 * zz - xx - yy) * vec3(splat_color.colors[base + 33], splat_color.colors[base + 34], splat_color.colors[base + 35]) +
//    SH_C3_3 * z * (2.0 * zz - 3.0 * xx - 3.0 * yy) * vec3(splat_color.colors[base + 36], splat_color.colors[base + 37], splat_color.colors[base + 38]) +
//    SH_C3_4 * x * (4.0 * zz - xx - yy) * vec3(splat_color.colors[base + 39], splat_color.colors[base + 40], splat_color.colors[base + 41]) +
//    SH_C3_5 * z * (xx - yy) * vec3(splat_color.colors[base + 42], splat_color.colors[base + 43], splat_color.colors[base + 44]) +
//    SH_C3_6 * x * (xx - 3.0 * yy) * vec3(splat_color.colors[base + 45], splat_color.colors[base + 46], splat_color.colors[base + 47]);

    OUT.color.rgb = clamp(0.5 + final_color, 0.0, 1.0);
}