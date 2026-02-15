#pragma once
#include <cstdint>

enum class PresentationImageType : uint8_t
{
    None,
    SwapChain,
    Depth,
    Stencil,
    DepthStencil,
};
