#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shading_language_include : require

#include "../common/gaussian_common.glsl"

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;


void main() {
    uint idx = gl_GlobalInvocationID.x;
    
    // Example access:
    // if (uint64_t(pc.splat_positions_address) != 0) {
    //     vec4 pos = pc.splat_positions_address.positions[idx];
    // }
}
