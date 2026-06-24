param (
    [switch]$Help,
    [switch]$Clean,
    [switch]$Rebuild,
    [ValidateSet("x64", "Win32", "ARM64")]
    [string]$Arch,
    [ValidateSet("Debug", "Release")]
    [string]$Config
)

function Show-Help {
    Write-Host @"
Usage: ./Build.ps1 [options]

Options:
  -Help, -h      Display this help menu.
  -Clean         Remove 'build' and 'Bin' directories.
  -Rebuild       Perform a clean build followed by a fresh compilation.
  -Arch <val>    Specify architecture (x64, Win32, ARM64).
  -Config <val>  Specify build config (Debug, Release).
  (no args)      Run standard compilation for all architectures and configs.
"@ -ForegroundColor Yellow
}

if ($Help) { Show-Help; exit }
if ($Rebuild) { $Clean = $true }

if ($Clean) {
    Write-Host "--- Cleaning build and Bin directories ---" -ForegroundColor Red
    if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
    if (Test-Path "Bin") { Remove-Item -Recurse -Force "Bin" }
    if (!$Rebuild -and !$Arch -and !$Config) { exit }
}

$ErrorActionPreference = "Stop"
$rootBin = "Bin"
if (!(Test-Path $rootBin)) { New-Item -ItemType Directory -Path $rootBin | Out-Null }

$architectures = if ($Arch) { @($Arch) } else { @("x64", "Win32", "ARM64") }
$configs = if ($Config) { @($Config) } else { @("Debug", "Release") }

foreach ($arch in $architectures) {
    $buildDir = "build/$arch"
    Write-Host "--- Configuring for $arch ---" -ForegroundColor Cyan
	
    cmake -S . -B $buildDir -G "Visual Studio 17 2022" -A $arch   
    
    foreach ($cfg in $configs) {
        Write-Host "--- Building $arch $cfg ---" -ForegroundColor Yellow
        cmake --build $buildDir --config $cfg
        
        $targetDir = "$rootBin/$arch"
        if (!(Test-Path $targetDir)) { New-Item -ItemType Directory -Path $targetDir | Out-Null }
        
        # Copy logic
        $srcPath = "$buildDir/bin/$cfg"
        if ($cfg -eq "Debug") {
            if (Test-Path "$srcPath/StackyManager_debug.exe") { Copy-Item "$srcPath/StackyManager_debug.exe" -Destination "$targetDir/" -Force }
            if (Test-Path "$srcPath/StackyManager_debug.pdb") { Copy-Item "$srcPath/StackyManager_debug.pdb" -Destination "$targetDir/" -Force }
        } else {
            if (Test-Path "$srcPath/StackyManager.exe") { Copy-Item "$srcPath/StackyManager.exe" -Destination "$targetDir/" -Force }
        }
    }
    Write-Host "--- $arch build complete and deployed ---" -ForegroundColor Green
}

Write-Host "--- Build process completed ---" -ForegroundColor Cyan