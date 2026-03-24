@echo off
setlocal

:: ---------------------------------------------------------------------------
:: compile_shaders.bat  —  Recompile Vulkan geometry shaders and update
::                          embedded SPIR-V headers.
::
:: Requirements:
::   - Vulkan SDK installed with VULKAN_SDK environment variable set
::   - Python 3 on PATH
::
:: Run this from the repository root, then rebuild the project in Visual Studio.
:: ---------------------------------------------------------------------------

set REPO=%~dp0
set GLSLC=

:: Locate glslc
if defined VULKAN_SDK (
    set GLSLC=%VULKAN_SDK%\Bin\glslc.exe
)
if not exist "%GLSLC%" (
    echo [ERROR] glslc not found at %VULKAN_SDK%\Bin\glslc.exe
    echo         Please install the Vulkan SDK and ensure VULKAN_SDK is set.
    exit /b 1
)

set VERT_SRC=%REPO%assets\shaders\geometry\geometry.vert
set FRAG_SRC=%REPO%assets\shaders\geometry\geometry.frag
set VERT_SPV=%REPO%geometry.vert.spv
set FRAG_SPV=%REPO%geometry.frag.spv
set VERT_HDR=%REPO%engine\renderer\Private\GeometryVertSPIRV.h
set FRAG_HDR=%REPO%engine\renderer\Private\GeometryFragSPIRV.h
set PY_SCRIPT=%REPO%scripts\spv_to_header.py

echo [1/4] Compiling geometry.vert ...
"%GLSLC%" "%VERT_SRC%" -o "%VERT_SPV%"
if errorlevel 1 ( echo [FAIL] vertex shader compilation failed & exit /b 1 )

echo [2/4] Compiling geometry.frag ...
"%GLSLC%" "%FRAG_SRC%" -o "%FRAG_SPV%"
if errorlevel 1 ( echo [FAIL] fragment shader compilation failed & exit /b 1 )

echo [3/4] Generating GeometryVertSPIRV.h ...
python "%PY_SCRIPT%" "%VERT_SPV%" k_geometryVertSPIRV "%VERT_HDR%"
if errorlevel 1 ( echo [FAIL] header generation failed & exit /b 1 )

echo [4/4] Generating GeometryFragSPIRV.h ...
python "%PY_SCRIPT%" "%FRAG_SPV%" k_geometryFragSPIRV "%FRAG_HDR%"
if errorlevel 1 ( echo [FAIL] header generation failed & exit /b 1 )

del "%VERT_SPV%" "%FRAG_SPV%"

echo.
echo Done. Rebuild the project in Visual Studio to pick up the new shaders.
endlocal
