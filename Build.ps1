param (
    [ValidateSet("x64", "x86", "arm64")] [string[]]$Arch = @("x64", "x86", "arm64"),
    [ValidateSet("Debug", "Release")] [string[]]$Config = @("Debug", "Release"),
    [ValidateSet("CMake", "Ninja")] [string]$Method = "CMake",
    [switch]$Clean,
	[switch]$Verify,
	[switch]$BuildTools,
    [alias('h')][switch]$Help
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Get-Item $PSScriptRoot
$DistBaseDir = Join-Path $ProjectRoot "dist"
$MethodDistDir = Join-Path $DistBaseDir $Method
$LogDir = Join-Path $MethodDistDir "logs"
$VCVars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$Timestamp = Get-Date -Format "yyyyMMdd-HHmm"

if (-not (Test-Path $LogDir)) { New-Item -ItemType Directory -Path $LogDir -Force | Out-Null }

if ($Clean) {
    # 1. Standard build artifacts
    if (Test-Path $DistBaseDir) { Remove-Item -Recurse -Force $DistBaseDir }
    if (Test-Path "build_artifacts") { Remove-Item -Recurse -Force "build_artifacts" }
    
    # 2. Targeted cleanup for VS, Bin, and Obj
    $TargetFolders = @(".vs", "bin", "obj")
    
    Write-Host "Searching for and removing build artifacts (bin, obj, .vs)..." -ForegroundColor Yellow
    
    Get-ChildItem -Path $ProjectRoot -Recurse -Directory -Include $TargetFolders -Force -ErrorAction SilentlyContinue | ForEach-Object {
        Write-Host "Removing: $($_.FullName)" -ForegroundColor Gray
        Remove-Item -Recurse -Force $_.FullName
    }
    
    Write-Host "Full clean complete." -ForegroundColor Green
    return
}

function Get-BinaryArch {
    param([string]$FilePath)
    if (-not (Test-Path $FilePath)) { return "NotFound" }
    $fs = [System.IO.File]::OpenRead($FilePath)
    $br = New-Object System.IO.BinaryReader($fs)
    try {
        $fs.Seek(0x3C, [System.IO.SeekOrigin]::Begin)
        $peHeaderOffset = $br.ReadInt32()
        $fs.Seek($peHeaderOffset + 4, [System.IO.SeekOrigin]::Begin)
        $machine = $br.ReadUInt16()
        # Check Architecture
		if ($machine -eq 0x014c) { return "x86" }
		if ($machine -eq 0x8664) { return "x64" }
		if ($machine -eq 0xaa64) { return "ARM64" }
    
		return "Unknown ($machine)"
    } finally { $br.Dispose(); $fs.Dispose() }
}

if ($Help) {
    Write-Host @"
Usage: .\Build.ps1 [Arguments]

Core Build Arguments:
  -Clean        Performs a cleanup of build artifacts.
  -BuildTools   Triggers 'BuildTools.ps1' with the -rebuild flag to compile 
                the GUI utilities for x64, x86, and ARM64.
  -Verify       Runs the architecture verification function on the final 
                binaries in the 'dist' directory.
  -Help         Displays this help documentation.

Build Configuration Parameters:
  -Arch         Filter build by architecture (Options: x64, x86, arm64).
                Default: All architectures are built.
  -Config       Filter build by configuration (Options: Debug, Release).
                Default: Both configurations are built.
  -Method       Sets the build system generator (Options: CMake, Ninja).
                Default: CMake.

Notes:
  - Default behavior (no arguments) executes a standard cmake build for all supported architectures.
  - You can combine flags (e.g., .\Build.ps1 -Clean -BuildTools) to clean and rebuild tools.
  - You can build for a single architecture (e.g., .\Build.ps1 -Method Ninja -Arch x64 -Config Debug) to build using Ninja an x64/Debug Executable.
  - Ninja can only build for x86/x64 architectures both release/Debug, for fast ittirative builds.
  - Only release builds are zipped in appropriate the output folder
  - BuildTools.ps1 also produces zipped Deployments for release in the relevant folder.
"@
    exit
}

if ($BuildTools) {
    Write-Host ">>> Invoking BuildTools.ps1 with -rebuild..." -ForegroundColor Magenta
    & .\BuildTools.ps1 -rebuild
    if ($LASTEXITCODE -ne 0) {
        Write-Error "BuildTools.ps1 failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
	exit
}

if ($Verify) {
    Write-Host "--- Running Architecture Verification on 'dist' ---" -ForegroundColor Cyan
    $exes = Get-ChildItem -Path $DistDir -Filter *.exe -Recurse
    if ($exes.Count -eq 0) {
        Write-Warning "No executables found in '$DistDir'. Please build first!"
    } else {
        foreach ($exe in $exes) {
            $BinaryArch = Get-BinaryArch -FilePath $exe.FullName
            Write-Host "$($exe.Name) [$($exe.Directory.Name)] -> $BinaryArch" -ForegroundColor Green
        }
    }
    exit
}

# --- Prerequisite Validation ---
Write-Host "--- Validating Build Environment ---" -ForegroundColor Cyan
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
if (-not $vs) { throw "Visual Studio with C++ workload not found." }
$VCVars = Join-Path $vs[0].installationPath "VC\Auxiliary\Build\vcvarsall.bat"

foreach ($A in $Arch) {
	if ($Method -eq "Ninja" -and $A -eq "arm64") {
        Write-Host "Skipping Ninja build for arm64 (Unsupported)." -ForegroundColor Gray
        continue
    }
    foreach ($C in $Config) {
        $BuildDir = Join-Path $ProjectRoot "build_artifacts\$Method\$A\$C"
        $LogFile = Join-Path $LogDir "stacky_$Method`_$A`_$C`_$Timestamp.log"
        $TargetDir = Join-Path $MethodDistDir "$A\$C"
        
        if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
        New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
        
        Write-Host "`n>>> Building $A | $C using $Method" -ForegroundColor Yellow

        if ($Method -eq "CMake") {
            $gen = "Visual Studio 17 2022"
            $archFlag = if ($A -eq "x86") { "-A Win32" } else { "-A $A" }
            $FullCmd = "call `"$VCVars`" $A && " +
                       "cmake -S `"$ProjectRoot`" -B `"$BuildDir`" -G `"$gen`" $archFlag && " +
                       "cmake --build `"$BuildDir`" --config $C --verbose 2>&1 > `"$LogFile`""
        } else {
            $VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
            $VsCMake = "$VSPath\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
            $VsNinja = "$VSPath\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
            $FullCmd = "set PATH=$VsCMake;$VsNinja;%PATH% && " +
                       "call `"$VCVars`" $A && " +
                       "cmake -S `"$ProjectRoot`" -B `"$BuildDir`" -G Ninja -DCMAKE_CXX_COMPILER=cl -DCMAKE_C_COMPILER=cl -DCMAKE_LINKER=link -DCMAKE_BUILD_TYPE=$C && " +
                       "ninja -C `"$BuildDir`" -v 2>&1 > `"$LogFile`""
        }

        $Proc = Start-Process -FilePath "cmd.exe" -ArgumentList "/c", $FullCmd -PassThru -NoNewWindow
        
        # Timeout/Hang Guard
        $Timeout = 120
        $Elapsed = 0
        while (!$Proc.HasExited -and $Elapsed -lt $Timeout) {
            Start-Sleep -Seconds 2
            $Elapsed += 2
        }
        if (!$Proc.HasExited) {
            Write-Warning "Build hung, forcing cleanup..."
            Stop-Process -Id $Proc.Id -Force
            Get-Process -Name "link" -ErrorAction SilentlyContinue | Stop-Process -Force
            continue
        }
        
        if ($Proc.ExitCode -ne 0) {
            Write-Error "Build failed. Exit: $($Proc.ExitCode). See $LogFile"
            continue
        }

        $ExePath = Get-ChildItem -Path $BuildDir -Filter "stacky.exe" -Recurse | Select-Object -First 1 -ExpandProperty FullName
        if ($ExePath) {
            New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
            Copy-Item -Path $ExePath -Destination (Join-Path $TargetDir "stacky.exe") -Force
            if ($C -eq "Release") {
                Compress-Archive -Path $ExePath -DestinationPath (Join-Path $TargetDir "stacky-$A.zip") -Force
            }
            Write-Host "Deployed: $A/$C ($(Get-BinaryArch -FilePath $ExePath))" -ForegroundColor Green
        }
    }
}