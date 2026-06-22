param(
    [switch]$Verify,
    [switch]$Clean,
    [switch]$Rebuild,
    [alias('h')][switch]$Help
)

$ProjectFile = "Tools\GUI\Setup_Stacky\Setup_Stacky.csproj"
$DistTools = "dist\Tools\Setup_Stacky"
$Architectures = @("win-x64", "win-x86", "win-arm64")

# --- Documentation ---
if ($Help) {
    Write-Host @"
Usage: .\BuildTools.ps1 [Arguments]

Arguments:
  (None)    Builds all architectures and runs verification.
  -Clean    Removes the '$DistTools' directory and exits.
  -Rebuild  Performs a clean, followed by a full build and verification.
  -Verify   Runs architecture checks on existing builds without rebuilding.
  -Help     Displays this help documentation.
"@
    exit
}

# --- Architecture Verification Function ---
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
    } finally { $br.Dispose(); $fs.Dispose() }
    
    # Check Architecture
    if ($machine -eq 0x014c) { return "x86" }
    if ($machine -eq 0x8664) { return "x64" }
    if ($machine -eq 0xaa64) { return "ARM64" }
    
    return "Unknown ($machine)"
}

# --- Execution Logic ---

# 1. Clean Operation
if ($Clean -or $Rebuild) {
    Write-Host "--- Cleaning build directory: $DistTools ---" -ForegroundColor Yellow
    if (Test-Path $DistTools) {
        Remove-Item -Path $DistTools -Recurse -Force
        Write-Host "Directory cleared." -ForegroundColor Green
    }
    # If strictly -Clean, exit here
    if ($Clean -and -not $Rebuild) { exit }
}

# 2. Build Operation
if (-not $Verify) {
    foreach ($Rid in $Architectures) {
        Write-Host "`n>>> Building GUI Utility for $Rid..." -ForegroundColor Cyan
        $TargetDir = Join-Path $DistTools $Rid
        if (-not (Test-Path $TargetDir)) { New-Item -ItemType Directory -Path $TargetDir -Force }
        
        dotnet publish $ProjectFile -c Release -r $Rid --self-contained true `
            -p:PublishSingleFile=true -p:PublishReadyToRun=false -o $TargetDir
        
        $ZipPath = Join-Path $TargetDir "Setup_Stacky_$Rid.zip"
        if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
        Compress-Archive -Path "$TargetDir\*" -DestinationPath $ZipPath -Force
        Write-Host "Deployed & Compressed: $Rid" -ForegroundColor Green
    }
}

# 3. Verification Operation
if ($Verify -or $Rebuild -or (-not $Clean -and -not $Rebuild -and -not $Verify)) {
    Write-Host "`n--- Running Architecture Verification on Tools ---" -ForegroundColor Cyan
    foreach ($Rid in $Architectures) {
        $ExePath = Join-Path "$DistTools\$Rid" "Setup_Stacky.exe"
        if (Test-Path $ExePath) {
            Write-Host "$Rid -> $(Get-BinaryArch -FilePath $ExePath)"
        } else {
            Write-Host "$Rid -> Not Found" -ForegroundColor Red
        }
    }
}