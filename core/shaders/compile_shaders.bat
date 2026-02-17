@echo off
setlocal enabledelayedexpansion

:: Validate input - at least 2 arguments required
if "%~2"=="" (
    echo Usage: %~nx0 path_to_shader_folder path_to_compiler_exe [path_to_include_dir] [copy_destination_path]
    echo.
    echo Example: %~nx0 ./shaders C:/VulkanSDK/bin/glslangValidator.exe "" ./build/shaders
    exit /b 1
)

set "SHADER_DIR=%~1"
set "GLSLANG=%~2"
set "INCLUDE_DIR=%~3"
set "COPY_DEST=%~4"

if not exist "%GLSLANG%" (
    echo Error: Compiler not found at "%GLSLANG%"
    exit /b 1
)

:: Handle Include Directory (Optional Argument 3)
set "INCLUDE_FLAG="
if not "%INCLUDE_DIR%"=="" (
    if exist "%INCLUDE_DIR%" (
        set "INCLUDE_FLAG=-I"%INCLUDE_DIR%""
        echo Including headers from "%INCLUDE_DIR%"...
    ) else (
        :: If arg3 is not a valid folder, check if user skipped arg3 and meant arg3 to be the copy dest
        :: This is a simple fallback check, though strict ordering is safer.
        echo Warning: Include directory "%INCLUDE_DIR%" not found. Proceeding without it.
    )
)

echo Compiling Vulkan shaders in "%SHADER_DIR%"...
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
    echo !NAME! | findstr /i ".mesh.glsl" >nul && set "STAGE=mesh"
    echo !NAME! | findstr /i ".task.glsl" >nul && set "STAGE=task"
    echo !NAME! | findstr /i ".rgen.glsl" >nul && set "STAGE=rgen"
    echo !NAME! | findstr /i ".rchit.glsl" >nul && set "STAGE=rchit"
    echo !NAME! | findstr /i ".rmiss.glsl" >nul && set "STAGE=rmiss"

    if defined STAGE (
        echo [SPV] !NAME!
        "%GLSLANG%" -V -g -gVS --target-env vulkan1.4 -S !STAGE! "!FILE!" -o "!OUT!" !INCLUDE_FLAG!
        if errorlevel 1 (
            echo [ERROR] Failed to compile !FILE!
            exit /b 1
        )
    )
)

:: ---------------------------------------------------------
:: NEW SECTION: Copy Logic
:: ---------------------------------------------------------
if not "%COPY_DEST%"=="" (
    echo.
    echo Mirroring shaders to: "%COPY_DEST%"

    :: /S      : Copy subdirectories (but not empty ones)
    :: /E      : Copy subdirectories (including empty ones)
    :: /IS     : Include Same files (overwrites)
    :: /NJH /NJS /NDL /NFL : Keeps the output quiet/clean
    robocopy "%SHADER_DIR%" "%COPY_DEST%" /E /IS /IT /NJH /NJS /NDL /NC /NS /NP

    :: Robocopy exit codes 0-3 are successful copies/no changes
    if errorlevel 8 (
        echo [ERROR] Robocopy failed to copy files.
        exit /b 1
    ) else (
        echo Copy complete.
    )
)
:: ---------------------------------------------------------

echo "Done Compiling shaders."