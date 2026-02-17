#pragma once
#include <cstdint>

enum class ShaderObjectType : uint8_t
{
    None,
    OpaquePass,
    TranslucentPass,
    ScreenspacePass,

    //Compute Pass
    CullingComputePass,
    HistogramComputePass,
    SortComputePass,
};
