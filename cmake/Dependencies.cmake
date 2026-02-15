include(FetchContent)

find_package(Vulkan REQUIRED)

find_package(vk-bootstrap QUIET)
if(vk-bootstrap_FOUND)
    message(STATUS "Using vk-bootstrap via find_package")
endif()

if(NOT vk-bootstrap_FOUND)
	FetchContent_Declare(
		vk_bootstrap
		GIT_REPOSITORY "https://github.com/charles-lunarg/vk-bootstrap"
		GIT_TAG        v1.4.336
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	message(STATUS "Using vk-bootstrap via FetchContent")
	FetchContent_MakeAvailable(vk_bootstrap)
endif()

find_package(SDL3 CONFIG QUIET)
if(SDL3_FOUND)
    message(STATUS "Using SDL3 via find_package")
endif()
if(NOT SDL3_FOUND)
	FetchContent_Declare(
		SDL3
		GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
		GIT_TAG        "main"
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	message(STATUS "Using SDL3 via FetchContent")
	FetchContent_MakeAvailable(SDL3)
endif()

find_package(STB QUIET)
if(STB_FOUND)
    message(STATUS "Using STB via find_package")
endif()
if(NOT STB_FOUND)
	FetchContent_Declare(
		STB
		GIT_REPOSITORY "https://github.com/unrealsid/stb-cmake.git"
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
	)
	message(STATUS "Using STB via FetchContent")
	FetchContent_MakeAvailable(STB)
endif()

find_package(TINYPLY QUIET)
if(TINYPLY_FOUND)
	message(STATUS "Using TinyPly via find_package")
endif()
if(NOT TINYPLY_FOUND)
	FetchContent_Declare(
			TINYPLY
			GIT_REPOSITORY "https://github.com/unrealsid/tinyply-cmake.git"
			GIT_SHALLOW TRUE
			GIT_PROGRESS TRUE
	)
	message(STATUS "Using Tiny Ply via FetchContent")
	FetchContent_MakeAvailable(TINYPLY)
endif()

#imgui
find_package(Imgui QUIET)
if(Imgui_FOUND)
	message(STATUS "Using ImgUI via find_package")
endif()
if(NOT Imgui_FOUND)
	FetchContent_Declare(
			Imgui
			GIT_REPOSITORY "https://github.com/unrealsid/imgui-cmake.git"
			GIT_SHALLOW TRUE
			GIT_PROGRESS TRUE
	)
	message(STATUS "Using Imgui via FetchContent")
	FetchContent_MakeAvailable(Imgui)
endif()