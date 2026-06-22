<#
.SYNOPSIS
    Sets up a StackyNG folder, creates a default config if missing, and a desktop shortcut.

.DESCRIPTION
    - Ensures the target directory exists (creates if missing).
    - Validates stacky.exe path.
    - Generates a default stacky.json config if not present (or overwrites with -Force).
    - Creates a desktop shortcut pointing to stacky.exe with the folder as argument.

.PARAMETER StackDir
    Full path to the folder containing your .lnk files.

.PARAMETER StackyExe
    Full path to your stacky.exe executable.

.PARAMETER ShortcutName
    Optional. Name of the desktop shortcut (default: "Stacky_Apps" or folder name).

.PARAMETER Force
    Optional. Overwrites existing config/shortcut if they already exist.

.PARAMETER Verbose
    Optional. Displays detailed progress messages.

.EXAMPLE
    .\setup_stackyNG.ps1 -StackDir "C:\Stacks\Apps" -StackyExe "C:\Tools\stacky.exe" -ShortcutName "MyStacks" -Force -Verbose
#>

param (
    [Parameter(Mandatory=$true, HelpMessage="Enter the full path to your Stack folder")]
    [string]$StackDir,

    [Parameter(Mandatory=$true, HelpMessage="Enter the full path to stacky.exe")]
    [string]$StackyExe,

    [string]$ShortcutName,

    [switch]$Force,

    [switch]$Verbose
)

# 1. Validation & Folder Creation
if (!(Test-Path $StackDir)) {
    Write-Host "Creating directory: $StackDir" -ForegroundColor Cyan
    New-Item -ItemType Directory -Path $StackDir | Out-Null
} elseif ($Verbose) {
    Write-Host "Directory exists: $StackDir" -ForegroundColor Yellow
}

if (!(Test-Path $StackyExe)) {
    throw "The executable '$StackyExe' was not found."
}

# Default shortcut name if not provided
if (-not $ShortcutName) {
    $ShortcutName = "Stacky_" + (Split-Path $StackDir -Leaf)
}

# 2. Create Default stacky.json
$ConfigPath = Join-Path $StackDir "stacky.json"
if (!(Test-Path $ConfigPath) -or $Force) {
    $DefaultJson = @"
{
    "theme": "auto",
    "overrides": {
        "ExampleApp.lnk": {
            "name": "Custom Name",
            "args": "--verbose",
            "admin": false
        }
    }
}
"@
    $DefaultJson | Out-File -FilePath $ConfigPath -Encoding utf8 -Force
    Write-Host "Config written: $ConfigPath" -ForegroundColor Green
} elseif ($Verbose) {
    Write-Host "Config already exists: $ConfigPath (use -Force to overwrite)" -ForegroundColor Yellow
}

# 3. Create Shortcut
try {
    $Desktop = [Environment]::GetFolderPath("Desktop")
    $ShortcutPath = "$Desktop\$ShortcutName.lnk"

    if (Test-Path $ShortcutPath -and -not $Force) {
        Write-Host "Shortcut already exists: $ShortcutPath (use -Force to overwrite)" -ForegroundColor Yellow
    } else {
        $WshShell = New-Object -ComObject WScript.Shell
        $Link = $WshShell.CreateShortcut($ShortcutPath)
        
        $Link.TargetPath = $StackyExe
        $Link.Arguments = "`"$StackDir`""
        $Link.WorkingDirectory = Split-Path $StackyExe
        $Link.IconLocation = $StackyExe
        $Link.Save()

        # Release COM object
        [System.Runtime.Interopservices.Marshal]::ReleaseComObject($WshShell) | Out-Null

        Write-Host "Success: Desktop shortcut '$ShortcutName.lnk' created." -ForegroundColor Green
    }
}
catch {
    throw "Failed to create shortcut: $_"
}
