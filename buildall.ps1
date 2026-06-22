# buildAll.ps1
# Wrapper to orchestrate full clean, build, and verify sequence.

$ErrorActionPreference = "Stop"

Write-Host "[0/4] Verifying Build Tools..." -ForegroundColor Yellow
& .\VerifyBuildTools.ps1
if ($LASTEXITCODE -ne 0) { Write-Error "Build Tools missing requirements!"; exit 1 }

Write-Host "[1/4] Performing full cleanup..." -ForegroundColor Yellow
& .\Build.ps1 -Clean

Write-Host "[2/4] Starting full build process..." -ForegroundColor Yellow
& .\Build.ps1

Write-Host "[3/4] Running build tools..." -ForegroundColor Yellow
& .\Build.ps1 -BuildTools

Write-Host "`n--- Sequence Complete ---" -ForegroundColor Green