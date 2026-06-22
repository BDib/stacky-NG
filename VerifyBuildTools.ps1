# VerifyBuildTools.ps1
$VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$VSWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

Write-Host "--- Verifying Toolchain ---" -ForegroundColor Cyan

# 1. Verify CMake
$CMakePath = "$VSPath\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if (Test-Path $CMakePath) {
    Write-Host "[OK] CMake found: $CMakePath" -ForegroundColor Green
} else {
    Write-Warning "[FAIL] CMake not found in expected VS path."
}

# 2. Verify Ninja
$NinjaPath = "$VSPath\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
if (Test-Path $NinjaPath) {
    Write-Host "[OK] Ninja found: $NinjaPath" -ForegroundColor Green
} else {
    Write-Warning "[FAIL] Ninja not found in expected VS path."
}

# 3. Verify C++ Workload (Existing check)
$CppCheck = & $VSWhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] C++ Desktop Workload found." -ForegroundColor Green
} else {
    Write-Warning "[FAIL] C++ Desktop Workload NOT detected."
}

# 4. Verify .NET SDK (Existing check)
$DotnetCheck = & $VSWhere -latest -requires Microsoft.Net.Component.SDK.10.0
if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] .NET SDK components found." -ForegroundColor Green
} else {
    Write-Warning "[FAIL] .NET SDK components NOT detected."
}