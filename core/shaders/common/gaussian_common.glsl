#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shading_language_include : require

struct Interpolants
{
    vec3  position;     // World-space vertex position
    float depth;   // Z coordinate after applying the view matrix (larger = further away)

    vec4 color;
    float alpha;
    vec3 conic;
    vec2 coordxy;  // local coordinate in quad, unit in pixel

    vec3 view_direction;
    vec3 normal;
};

// --- Buffer Definitions ---
layout(buffer_reference, std430) readonly buffer CameraData
{
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 camera_position;
    vec4 hfovxy_focal;
};

layout(buffer_reference, std430) readonly buffer SplatID        { int gi[]; };
layout(buffer_reference, std430) readonly buffer SplatPosition  { vec4 positions[]; };
layout(buffer_reference, std430) readonly buffer SplatScale     { vec4 scales[]; };
layout(buffer_reference, std430) readonly buffer SplatColor     { float colors[]; };
layout(buffer_reference, std430) readonly buffer SplatQuat      { vec4 quats[]; };
layout(buffer_reference, std430) readonly buffer SplatAlpha     { float alphas[]; };

layout(buffer_reference, std430) readonly buffer SplatRenderParams
{
    //x -> SIGMA
    //y -> v_i
    //z -> depth multiplier
    //w -> power curve
    vec4 params;
};

layout(push_constant) uniform PushConstants
{
    CameraData camera_data_adddress;
    SplatID splat_ids_address;
    SplatPosition splat_positions_address;
    SplatScale splat_scales_address;
    SplatColor splat_colors_address;
    SplatQuat splat_quats_address;
    SplatAlpha splat_alphas_address;
    SplatRenderParams splat_render_params_address;
} pc;