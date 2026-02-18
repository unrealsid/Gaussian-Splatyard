#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shading_language_include : require

#pragma optimize(off)

#define WORKGROUP_SIZE 512
layout (local_size_x = WORKGROUP_SIZE) in;

layout(buffer_reference, std430) readonly buffer CameraData
{
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 camera_position;
    vec4 hfovxy_focal;
};

//The buffer that is sent to the render pass
layout(buffer_reference, std430) readonly buffer VisibleSplatID { uvec2 visible_gi[]; };
layout(buffer_reference, std430) buffer VisibleCount            { uint visible_count; };

layout(buffer_reference, std430) readonly buffer SplatID        { int gi[]; };
layout(buffer_reference, std430) readonly buffer SplatPosition  { vec4 positions[]; };

layout(push_constant) uniform PushConstantsComputeCulling
{
    CameraData camera_data_adddress;
    SplatID splat_ids_address;
    SplatPosition splat_positions_address;

    VisibleSplatID visible_splat_ids_address;
    VisibleCount visible_count_address;
    uint max_splats;
} pc_compute_culling;

void main()
{
    uint idx = gl_GlobalInvocationID.x;

    if(idx > pc_compute_culling.max_splats - 2)
    {
        return;
    }

    CameraData camera_matrices = pc_compute_culling.camera_data_adddress;
    SplatID splat_ids = pc_compute_culling.splat_ids_address;
    SplatPosition splat_positions = pc_compute_culling.splat_positions_address;
    VisibleSplatID visible_gis = pc_compute_culling.visible_splat_ids_address;
    VisibleCount visible_count_addr = pc_compute_culling.visible_count_address;

    int splat_id = splat_ids.gi[idx];
    vec4 g_pos = splat_positions.positions[splat_id];

    vec4 g_pos_view = camera_matrices.view_matrix * g_pos;
    vec4 g_pos_screen = camera_matrices.projection_matrix * g_pos_view;

    // Perspective divide
    g_pos_screen.xyz = g_pos_screen.xyz / g_pos_screen.w;

    // Early culling
    if (any(greaterThan(abs(g_pos_screen.xyz), vec3(1.3))))
    {
        //gl_Position = vec4(-100, -100, -100, 1);
        return;
    }

    float dist = g_pos_screen.z;
    uint out_idx = atomicAdd(visible_count_addr.visible_count, 1);

    //Convert float to uint to sort
    visible_gis.visible_gi[idx] = uvec2(uint((1 - dist * 0.5 + 0.5) * 0xFFFF), gl_GlobalInvocationID.x);
}
