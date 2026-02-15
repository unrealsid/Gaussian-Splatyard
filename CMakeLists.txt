cmake_minimum_required(
	VERSION 4.0.0 
	FATAL_ERROR
)
project(
    Vk-GaussSplatyardEngine
	LANGUAGES CXX
    VERSION 0.0.1
)

include(cmake/Dependencies.cmake)
include(GNUInstallDirs)

add_subdirectory(core)

target_compile_features(Vk_GaussianSplatyardEngine PRIVATE cxx_std_20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()