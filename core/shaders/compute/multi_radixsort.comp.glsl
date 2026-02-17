/**
* VkRadixSort written by Mirco Werner: https://github.com/MircoWerner/VkRadixSort
* Based on implementation of Intel's Embree: https://github.com/embree/embree/blob/v4.0.0-ploc/kernels/rthwif/builder/gpu/sort.h
*/

#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_KHR_shader_subgroup_basic: enable
#extension GL_KHR_shader_subgroup_arithmetic: enable
#extension GL_KHR_shader_subgroup_ballot: enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shading_language_include : require

#define WORKGROUP_SIZE 256// assert WORKGROUP_SIZE >= RADIX_SORT_BINS
#define RADIX_SORT_BINS 256
#define SUBGROUP_SIZE 32// 32 NVIDIA; 64 AMD

layout (local_size_x = WORKGROUP_SIZE) in;

layout (buffer_reference, std430) buffer Elements_in {
    uint g_elements_in[];
};

layout (buffer_reference, std430) buffer Elements_out {
    uint g_elements_out[];
};

layout (buffer_reference, std430) buffer Histograms {
// [histogram_of_workgroup_0 | histogram_of_workgroup_1 | ... ]
    uint g_histograms[];// |histogram_addr.g_histograms| = RADIX_SORT_BINS * #WORKGROUPS = RADIX_SORT_BINS * g_num_workgroups
};

layout(push_constant, std430) uniform PushConstantsRadixSort
{
    uint g_num_elements; // == NUM_ELEMENTS
    uint g_shift;
    uint g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
    uint g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP

    Elements_in elements_in_address;
    Elements_out elements_out_address;
    Histograms histograms_address;
} pc_radix_sort;

shared uint[RADIX_SORT_BINS / SUBGROUP_SIZE] sums;// subgroup reductions
shared uint[RADIX_SORT_BINS] global_offsets;// global exclusive scan (prefix sum)

struct BinFlags {
    uint flags[WORKGROUP_SIZE / 32];
};

shared BinFlags[RADIX_SORT_BINS] bin_flags;

void main() {
    uint gID = gl_GlobalInvocationID.x;
    uint lID = gl_LocalInvocationID.x;
    uint wID = gl_WorkGroupID.x;
    uint sID = gl_SubgroupID;
    uint lsID = gl_SubgroupInvocationID;

    uint local_histogram = 0;
    uint prefix_sum = 0;
    uint histogram_count = 0;

    Elements_in e_in_addr = pc_radix_sort.elements_in_address;
    Elements_out e_out_addr = pc_radix_sort.elements_out_address;
    Histograms histogram_addr = pc_radix_sort.histograms_address;

    if (lID < RADIX_SORT_BINS) {
        uint count = 0;
        for (uint j = 0; j < pc_radix_sort.g_num_workgroups; j++) {
            const uint t = histogram_addr.g_histograms[RADIX_SORT_BINS * j + lID];
            local_histogram = (j == wID) ? count : local_histogram;
            count += t;
        }
        histogram_count = count;
        const uint sum = subgroupAdd(histogram_count);
        prefix_sum = subgroupExclusiveAdd(histogram_count);
        if (subgroupElect()) {
            // one thread inside the warp/subgroup enters this section
            sums[sID] = sum;
        }
    }
    barrier();

    if (lID < RADIX_SORT_BINS) {
        const uint sums_prefix_sum = subgroupBroadcast(subgroupExclusiveAdd(sums[lsID]), sID);
        const uint global_histogram = sums_prefix_sum + prefix_sum;
        global_offsets[lID] = global_histogram + local_histogram;
    }

    //     ==== scatter keys according to global offsets =====
    const uint flags_bin = lID / 32;
    const uint flags_bit = 1 << (lID % 32);

    for (uint index = 0; index < pc_radix_sort.g_num_blocks_per_workgroup; index++) {
        uint elementId = wID * pc_radix_sort.g_num_blocks_per_workgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lID;

        // initialize bin flags
        if (lID < RADIX_SORT_BINS) {
            for (int i = 0; i < WORKGROUP_SIZE / 32; i++) {
                bin_flags[lID].flags[i] = 0U;// init all bin flags to 0
            }
        }
        barrier();

        uint element_in = 0;
        uint binID = 0;
        uint binOffset = 0;
        if (elementId < pc_radix_sort.g_num_elements) {
            element_in = e_in_addr.g_elements_in[elementId];
            binID = uint(element_in >> pc_radix_sort.g_shift) & uint(RADIX_SORT_BINS - 1);
            // offset for group
            binOffset = global_offsets[binID];
            // add bit to flag
            atomicAdd(bin_flags[binID].flags[flags_bin], flags_bit);
        }
        barrier();

        if (elementId < pc_radix_sort.g_num_elements) {
            // calculate output index of element
            uint prefix = 0;
            uint count = 0;
            for (uint i = 0; i < WORKGROUP_SIZE / 32; i++) {
                const uint bits = bin_flags[binID].flags[i];
                const uint full_count = bitCount(bits);
                const uint partial_count = bitCount(bits & (flags_bit - 1));
                prefix += (i < flags_bin) ? full_count : 0U;
                prefix += (i == flags_bin) ? partial_count : 0U;
                count += full_count;
            }
            e_out_addr.g_elements_out[binOffset + prefix] = element_in;
            if (prefix == count - 1) {
                atomicAdd(global_offsets[binID], count);
            }
        }

        barrier();
    }
}