#pragma once
#include <cstdint>
#include <string>

constexpr uint32_t window_width = 1920;
constexpr uint32_t window_height = 1080;
constexpr const char* window_title = "Vulkan Renderer";
const std::string shader_root_path = R"(C:\Users\Sid\Documents\Visual Studio 18\Code\GaussianSplatViewer\Vk_GaussianSplatViewer\shaders\)";

//Maximum number of objects the engine supports
constexpr uint32_t max_object_count = 100;