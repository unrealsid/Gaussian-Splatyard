set(
    Header_Files
	"include/vulkanapp/feature_activator/VulkanFeatureActivator.h"

	"include/structs/EngineContext.h"
	"include/structs/GPU_Buffer.h"
	"include/structs/LoadedImageData.h"
	"include/structs/scene/PushConstantBlock.h"
	"include/structs/scene/CameraData.h"
	"include/structs/WindowCreateParams.h"
	"include/structs/geometry/Vertex.h"
	"include/structs/geometry/Vertex2D.h"
	"include/structs/geometry/GaussianSurface.h"
	"include/structs/geometry/GaussianSurface.cpp"
	"include/structs/Vk_Image.h"
	"include/structs/EngineRenderTargets.h"
	
	"include/platform/WindowManager.h"
	"include/platform/input/InputManager.h"

	"include/core/Engine.h"
	"include/config/Config.inl"

	"include/3d/ModelUtils.h"
	"include/3d/GaussianSplatPlyLoader.h"

	"include/enums/PresentationImageType.h"
	"include/enums/inputs/InputAction.h"
	"include/enums/inputs/UIAction.h"

	"include/materials/Material.h"
	"include/materials/MaterialUtils.h"
	"include/materials/ShaderObject.h"

	"include/renderer/subpasses/ForwardGeometryPass.h"
	"include/renderer/subpasses/ImGuiPass.h"
	"include/renderer/subpasses/TranslucencyPass.h"
	"include/renderer/subpasses/ScreenspacePass.h"
	"include/renderer/subpasses/ComputePass.h"
	"include/renderer/DrawState.h"

	"include/renderer/Renderer.h"
	"include/renderer/RenderPass.h"
	"include/renderer/Subpass.h"
	"include/renderer/GPU_BufferContainer.h"
	"include/camera/FirstPersonCamera.h"

	"include/platform/input/UIActionManager.h"
	"include/common/Types.h"
	"include/3d/Scene.h"
	"include/enums/BufferAllocationType.h"
)
source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" 
	PREFIX "Header Files" 
	FILES ${Header_Files}
)

set(
    Source_Files
	"source/vulkanapp/DeviceManager.cpp"
	"source/vulkanapp/SwapchainManager.cpp"
	"source/vulkanapp/VulkanCleanupQueue.cpp"
	"source/vulkanapp/feature_activator/VulkanFeatureActivator.cpp"

	"source/platform/WindowManager.cpp"
	"source/platform/input/InputManager.cpp"
	"source/platform/input/UIActionManager.cpp"

	"source/core/Engine.cpp"

	"source/vulkanapp/utils/DescriptorUtils.cpp"
	"source/vulkanapp/utils/FileUtils.cpp"
	"source/vulkanapp/utils/ImageUtils.cpp"
	"source/vulkanapp/utils/MemoryUtils.cpp"
	"source/vulkanapp/utils/RenderUtils.cpp"
	"source/vulkanapp/utils/Vk_Utils.cpp"

	"source/3d/ModelUtils.cpp"
	"source/3d/GaussianSplatPlyLoader.cpp"

	"source/materials/ShaderObject.cpp"
	"source/materials/MaterialUtils.cpp"
	"source/materials/Material.cpp"

	"source/renderer/subpasses/ForwardGeometryPass.cpp"
	"source/renderer/subpasses/ImGuiPass.cpp"
	"source/renderer/subpasses/TranslucencyPass.cpp"
	"source/renderer/subpasses/ScreenspacePass.cpp"
	"source/renderer/subpasses/ComputePass.cpp"
	"source/renderer/DrawState.cpp"

	"source/renderer/Renderer.cpp"
	"source/renderer/Subpass.cpp"
	"source/renderer/RenderPass.cpp"
	"source/renderer/RendererVulkan.cpp"
	"source/renderer/GPU_BufferContainer.cpp"

	"source/main.cpp"
	"source/camera/FirstPersonCamera.cpp"
	"source/3d/Scene.cpp"
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"
    PREFIX "Source Files" 
    FILES ${Source_Files}
)

set(
    ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

find_package(Vulkan REQUIRED)

set(ENGINE_PROJECT_NAME "Vk_GaussianSplatViewer")

add_executable( ${ENGINE_PROJECT_NAME} ${ALL_FILES})

target_include_directories(
    ${ENGINE_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${ENGINE_PROJECT_NAME} PUBLIC Vulkan::Vulkan
													vk-bootstrap
													SDL3-shared
													STB-image
													tinyply
													imgui::imgui)