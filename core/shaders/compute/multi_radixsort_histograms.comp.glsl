/**
* VkRadixSort written by Mirco Werner: https://github.com/MircoWerner/VkRadixSort
* Based on implementation of Intel's Embree: https://github.com/embree/embree/blob/v4.0.0-ploc/kernels/rthwif/builder/gpu/sort.h
*/
#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shading_language_include : require

#pragma optimize(off)

#define WORKGROUP_SIZE 256 // assert WORKGROUP_SIZE >= RADIX_SORT_BINS
#define RADIX_SORT_BINS 256

layout (local_size_x = WORKGROUP_SIZE) in;

layout (buffer_reference, std430) buffer Elements_in {
    uint g_elements_in[];
};

layout (buffer_reference, std430) buffer Histograms {
// [histogram_of_workgroup_0 | histogram_of_workgroup_1 | ... ]
    uint g_histograms[];// |g_histograms| = RADIX_SORT_BINS * #WORKGROUPS = RADIX_SORT_BINS * g_num_workgroups
};

layout (push_constant, std430) uniform PushConstantsHistogram
{
    uint g_num_elements;
    uint g_shift;
    uint g_num_workgroups;
    uint g_num_blocks_per_workgroup;

    Elements_in elements_in_address;
    Histograms histograms_address;
} pc_histograms;

shared uint[RADIX_SORT_BINS] histogram;

void main() {
    uint gID = gl_GlobalInvocationID.x;
    uint lID = gl_LocalInvocationID.x;
    uint wID = gl_WorkGroupID.x;

    Elements_in e_in_addr = pc_histograms.elements_in_address;
    Histograms histogram_addr = pc_histograms.histograms_address;

    // initialize histogram
    if (lID < RADIX_SORT_BINS) {
        histogram[lID] = 0U;
    }
    barrier();

    for (uint index = 0; index < pc_histograms.g_num_blocks_per_workgroup; index++) {
        uint elementId = wID * pc_histograms.g_num_blocks_per_workgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lID;
        if (elementId < pc_histograms.g_num_elements) {
            // determine the bin
            const uint bin = uint(e_in_addr.g_elements_in[elementId] >> pc_histograms.g_shift) & uint(RADIX_SORT_BINS - 1);
            // increment the histogram
            atomicAdd(histogram[bin], 1U);
        }
    }
    barrier();

    if (lID < RADIX_SORT_BINS) {
        histogram_addr.g_histograms[RADIX_SORT_BINS * wID + lID] = histogram[lID];
    }
}