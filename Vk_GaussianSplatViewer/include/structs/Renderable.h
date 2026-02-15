#pragma once

#include "structs/GPU_Buffer.h"

//Defines an object that can be rendered
struct Renderable
{
    uint32_t object_index = 0;

    //A simple way to track translucent vs opaque materials.
    //TODO: Bring in a more robust material solution later
    uint32_t material_index = 0;

    GPU_Buffer vertex_buffer;
    uint32_t vertex_count = 0;

    GPU_Buffer index_buffer;

    GPU_Buffer model_matrix_buffer;
    GPU_Buffer vertex_colors_buffer;
};
