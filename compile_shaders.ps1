$ErrorActionPreference = "Stop"

$glslc  = "C:\VulkanSDK\1.4.341.1\Bin\glslc.exe"
$python = "python"
$repo   = "C:\Programming\arche"

$vertSrc  = "$repo\assets\shaders\geometry\geometry.vert"
$fragSrc  = "$repo\assets\shaders\geometry\geometry.frag"
$vertSpv  = "$repo\geometry.vert.spv"
$fragSpv  = "$repo\geometry.frag.spv"
$vertHdr  = "$repo\engine\renderer\Private\GeometryVertSPIRV.h"
$fragHdr  = "$repo\engine\renderer\Private\GeometryFragSPIRV.h"
$pyScript = "$repo\scripts\spv_to_header.py"

if (-not (Test-Path $glslc)) {
    Write-Error "glslc not found at $glslc. Install the Vulkan SDK."
}

Write-Host "[1/4] Compiling geometry.vert ..."
& $glslc $vertSrc -o $vertSpv
if ($LASTEXITCODE -ne 0) { Write-Error "Vertex shader compilation failed." }

Write-Host "[2/4] Compiling geometry.frag ..."
& $glslc $fragSrc -o $fragSpv
if ($LASTEXITCODE -ne 0) { Write-Error "Fragment shader compilation failed." }

Write-Host "[3/4] Generating GeometryVertSPIRV.h ..."
& $python $pyScript $vertSpv k_geometryVertSPIRV $vertHdr
if ($LASTEXITCODE -ne 0) { Write-Error "Header generation failed for vert." }

Write-Host "[4/4] Generating GeometryFragSPIRV.h ..."
& $python $pyScript $fragSpv k_geometryFragSPIRV $fragHdr
if ($LASTEXITCODE -ne 0) { Write-Error "Header generation failed for frag." }

Remove-Item $vertSpv, $fragSpv -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Done. Rebuild the project in Visual Studio."
