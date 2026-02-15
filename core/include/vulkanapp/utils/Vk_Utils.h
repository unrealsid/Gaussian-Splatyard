#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <VkBootstrapDispatch.h>

struct Vk_SceneData;
struct Init;

namespace utils
{
    //Used for debugging vulkan objects in render doc or nvidia nsight
    void set_vulkan_object_Name(const vkb::DispatchTable& disp, uint64_t object_handle, VkObjectType object_type, const std::string& name);
}
