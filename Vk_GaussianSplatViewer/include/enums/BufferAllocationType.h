#pragma once
#include <cstdint>

enum class BufferAllocationType : uint8_t
{
    None,
    VertexAllocationWithStaging,
    MappedAllocation,
};
