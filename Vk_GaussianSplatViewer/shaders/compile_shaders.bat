REM How to use this file:
REM ./compile_shaders.bat D:\Projects\CPP\Vk_GaussianSplat\Vk_GaussianSplatViewer\shaders C:\VulkanSDK\1.4.335.0\Bin\glslc.exe

@echo off
setlocal enabledelayedexpansion

:: Validate input - at least 2 arguments required
if "%~2"=="" (
    echo Usage: %~nx0 path_to_shader_folder path_to_glslangValidator_exe [path_to_include_dir]
    exit /b 1
)

set "SHADER_DIR=%~1"
set "GLSLANG=%~2"
set "INCLUDE_DIR=%~3"

if not exist "%GLSLANG%" (
    echo Error: glslangValidator not found at "%GLSLANG%"
    exit /b 1
)

:: Optional include directory
set "INCLUDE_FLAG="
if not "%INCLUDE_DIR%"=="" (
    if not exist "%INCLUDE_DIR%" (
        echo Error: include directory not found at "%INCLUDE_DIR%"
        exit /b 1
    )
    set "INCLUDE_FLAG=-I"%INCLUDE_DIR%""
    echo Including headers from "%INCLUDE_DIR%"...
) else (
    echo No include directory specified.
)

echo Compiling Vulkan shaders in "%SHADER_DIR%" using glslangValidator (debug enabled)...
echo.

:: Loop over all .glsl files recursively
for /R "%SHADER_DIR%" %%F in (*.glsl) do (
    set "FILE=%%F"
    set "NAME=%%~nxF"
    set "OUT=%%~dpnF.spv"

    :: Detect shader stage
    set "STAGE="
    echo !NAME! | findstr /i ".vert.glsl" >nul && set "STAGE=vert"
    echo !NAME! | findstr /i ".frag.glsl" >nul && set "STAGE=frag"
    echo !NAME! | findstr /i ".geom.glsl" >nul && set "STAGE=geom"
    echo !NAME! | findstr /i ".comp.glsl" >nul && set "STAGE=comp"
    echo !NAME! | findstr /i ".tesc.glsl" >nul && set "STAGE=tesc"
    echo !NAME! | findstr /i ".tese.glsl" >nul && set "STAGE=tese"

    if defined STAGE (
        echo [SPV] !FILE!
        "%GLSLANG%" -V --target-env vulkan1.3 -gVS -S !STAGE! "!FILE!" -o "!OUT!" !INCLUDE_FLAG!
        if errorlevel 1 (
            echo [ERROR] Failed to compile !FILE!
            exit /b 1
        )
    )
)

echo.
echo Done.