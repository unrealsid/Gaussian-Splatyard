#version 460
#extension GL_ARB_shading_language_include : require
#include "../common/gaussian_common.glsl"

layout(location = 0) in Interpolants IN;

layout (location = 0) out vec4 outColor;

void main()
{
    // 1. Compute 2D Gaussian Spatial Falloff
    // This calculates alpha decay based on distance from the splat center in screen space
    float power = -0.5f * (IN.conic.x * IN.coordxy.x * IN.coordxy.x + IN.conic.z * IN.coordxy.y * IN.coordxy.y) - IN.conic.y * IN.coordxy.x * IN.coordxy.y;

    if (power > 0.f)
        discard;

    float alpha_i = min(0.99f, IN.alpha * exp(power));

    // Cull low opacity
    if (alpha_i < 1.f / 255.f)
        discard;

    // Output to Render Target 0: Weighted Color Numerator
    outColor = vec4(IN.color.rgb, alpha_i);
}