#pragma once

#include <glm/glm.hpp>

struct SplatRenderingParams
{
    /*
    *   static float sigma = 0.0f;            // 0 -> 1000 (step 100)
        static float v_i = 0.0f;              // 0 -> 5
        static float depth_multiplier = 0.0f; // 0 -> 50
        static float power = 0.0f;            // 0 -> 10
     */

    glm::vec4 rendering_params;
};