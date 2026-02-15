#pragma once

#include <memory>
#include <VkBootstrapDispatch.h>

#include "3d/GaussianSplatPlyLoader.h"
#include "vulkanapp/DeviceManager.h"
#include "vulkanapp/SwapchainManager.h"
#include "platform/WindowManager.h"
#include "platform/input/InputManager.h"
#include "platform/input/UIActionManager.h"
#include "renderer/Renderer.h"
#include "renderer/GPU_BufferContainer.h"

struct EngineContext
{
    std::unique_ptr<vulkanapp::DeviceManager> device_manager;
    std::unique_ptr<platform::WindowManager> window_manager;
    std::unique_ptr<vulkanapp::SwapchainManager> swapchain_manager;
    std::unique_ptr<core::rendering::Renderer> renderer;

    std::unique_ptr<core::rendering::GPU_BufferContainer> buffer_container;

    std::unique_ptr<input::InputManager> input_manager;
    std::unique_ptr<ui::UIActionManager> ui_action_manager;

    vkb::InstanceDispatchTable instance_dispatch_table;
    vkb::DispatchTable dispatch_table;

    std::unique_ptr<splat_loader::GaussianSplatPlyLoader> splat_loader;
};
