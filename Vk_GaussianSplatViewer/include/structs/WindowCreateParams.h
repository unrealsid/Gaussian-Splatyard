#pragma once

#include <cstdint>

struct WindowCreateParams
{
    uint32_t window_width;;
    uint32_t window_height;

    const char* window_title;
};
