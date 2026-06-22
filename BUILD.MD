# Building StackyNG

StackyNG is designed for versatility, supporting multiple build systems to accommodate various developer environments.

[See Documentation](https://bdib.github.io/stacky-NG/docs/)

## 1. Supported Build Methods

### Method 0: PowerShell Automation (Recommended)

This script orchestrates compilation for multiple Windows 11 target architectures and configurations automatically.

* **Usage**: Run from a standard PowerShell terminal (no special Native Tools prompt required).

* **Build all targets**: `.\build.ps1`

* **Build specific targets**: `.\build.ps1 -Arch x64 -Config Release`

* **Clean & Rebuild**: `.\build.ps1 -Clean`

* **Get Help**: `.\build.ps1 -h OR -Help`

* **Parameters**:

* `-Method`: `CMake` or `Ninja`.

* `-Arch`: `x64`, `x86`, or `arm64`.

* `-Config`: `Release` or `Debug`.

* `-BuildTools`: Builds the Tools, [See Documentation](https://bdib.github.io/stacky-NG/docs/).

* `-Clean`: Wipe previous build artifacts. **Note** : -Clean removes .vs folders  in the project, ***Visual Studio*** often keeps the `.vs` folder locked until you fully close the IDE. If you get an "Access Denied" error during cleanup, it just means you have the project open in Visual Studio. Ensure your IDE is closed before running the `-Clean` command to guarantee all files (like `.suo` and `.db` database files) are successfully purged. 

### Method 1: CMake (Modern)

1. Open a **Developer Command Prompt**.
2. Create/Navigate: `mkdir build && cd build`.
3. Configure: `cmake .. -G "Visual Studio 17 2022" -A x64` (Replace `x64` with `Win32` or `ARM64`).
4. Build: `cmake --build . --config Release`.
5. Binary location: `build/bin/[arch]/stacky.exe`.

### Method 2: MSBuild (Direct)

1. Open a **Developer Command Prompt**.
2. Run: `msbuild vsproj\stacky.sln /p:Configuration=Release /p:Platform=x64` (Use `Platform=Win32` for 32-bit).

### Method 3: Visual Studio IDE

1. Open `vsproj/stacky.sln` in **Visual Studio 2022** (or 2026).
2. Set configuration to **Release** and target architecture (x64/x86/ARM64).
3. Right-click **StackyNG** project > **Build**.

### Method 4: NMake (Fast CLI)

For developers who prefer a lightweight command-line build without CMake, use the `Makefile` located in the `vsproj/` directory.

**Requirements**:

* Open the **Developer Command Prompt for VS 2026/2022**.
* Navigate to the `vsproj/` folder: `cd vsproj`.

**Usage**:
You must specify both `ARCH` and `CONFIG` to ensure your binaries are placed in the correct isolated subdirectories:

| Target            | Command                           |
| ----------------- | --------------------------------- |
| **x64 Debug**     | `nmake ARCH=x64 CONFIG=Debug`     |
| **x64 Release**   | `nmake ARCH=x64 CONFIG=Release`   |
| **x86 Debug**     | `nmake ARCH=x86 CONFIG=Debug`     |
| **ARM64 Release** | `nmake ARCH=arm64 CONFIG=Release` |

*Note: For `ARM64` targets, ensure you have the "MSVC v143 - VS 2022 C++ ARM64 build tools" workload installed via the Visual Studio Installer, and ensure you are using the "ARM64 Native Tools Command Prompt".*

**Cleaning Artifacts**:
To wipe all generated objects and binaries across all architectures:
`nmake clean`

### Method 5: Legacy Batch Script (`build.bat`)

1. Open any **Developer Command Prompt**.
2. Run: `build.bat` (Automatically detects architecture and outputs to `bin/[arch]/stacky.exe`).

---

## 2. Prerequisites & Dependencies

### System Requirements

* **OS**: Windows 11 (x64/ARM64).
* **Compiler**: Microsoft Visual C++ (MSVC) with C++17 support.
* **SDK**: Windows SDK 10.0 or 11.0.
* **Tools**: Visual Studio 2022 (recommended) or 2026.

### Automated Workload Setup

StackyNG requires the **"Desktop development with C++"** workload.

#### Option 1: Chocolatey

```powershell
# Build Tools (headless)
choco install visualstudio2022buildtools -y
choco install visualstudio2022-workload-vctools -y

# Full IDE & Native Desktop
choco install visualstudio2022-workload-nativedesktop -y
```

#### Option 2: Winget (Recommended)

```powershell
# Build Tools
winget install Microsoft.VisualStudio.2022.BuildTools --override "--passive --add Microsoft.VisualStudio.Workload.VCTools"

# Full IDE
winget install Microsoft.VisualStudio.2022.Community --override "--passive --add Microsoft.VisualStudio.Workload.NativeDesktop"

# Architecture Support
winget install Microsoft.VisualStudio.2022.Community --override "--passive --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.Tools.ARM64"
```

### Required Tooling

* **CMake**: `choco install cmake` OR `winget install Kitware.CMake`
* **Ninja (Optional)**: `choco install ninja` OR `winget install Ninja-build.Ninja`

*Note: Restart your terminal or open a **Developer Command Prompt** after installation to refresh environment variables.*

---

## 3. Linked Libraries

* **Core Win32**: `user32.lib`, `gdi32.lib`, `shell32.lib`, `ole32.lib`
* **UI/Controls**: `comctl32.lib`, `windowscodecs.lib` (WIC), `uxtheme.lib`, `dwmapi.lib`
* **System**: `advapi32.lib`

---

## 4. GUI Utility (`Tools/GUI/`)

The **Setup_Stacky** GUI is a subproject of the repository, compiled automatically as part of the `build.ps1` workflow.

* **Manual Build**: Navigate to `Tools/GUI/` and run: `dotnet build -c Debug`.
* **Output Location**: Automatically moved to `/dist/Tools/Setup_Stacky.exe` during the main build process.
* **Dependencies**: Requires **.NET 10.0 SDK**. If building on a fresh machine, run `winget install Microsoft.DotNet.SDK.10`.

**[See Documentation](https://bdib.github.io/stacky-NG/docs/)**

---

## 5. Troubleshooting

* **Unresolved Externals**: Ensure you are building from a **Developer Command Prompt**.
* **Multiple Definition Errors**: Ensure compiler is set to C++17 mode (`/std:c++17`).
* **Missing Icons**: Ensure `src/stacky.rc` is compiled and linked.
* **Workload Components**: `Microsoft.VisualStudio.Workload.VCTools` provides essential compiler/linker tools; `Microsoft.VisualStudio.Workload.NativeDesktop` includes full SDK support.

build.ps1 fails to run due to being unsigned, Execution Policy or is blocked.

### 1. Unblock the file (Most likely fix)

Even with `RemoteSigned` set, Windows often "blocks" files downloaded from the internet or created on other systems.

1. Right-click `Build.ps1` in File Explorer.
2. Select **Properties**.
3. At the bottom of the **General** tab, check the box that says **"Unblock"**.
4. Click **Apply** and **OK**.

### 2. Force the Execution Policy in the current session

Sometimes the `CurrentUser` scope isn't prioritized correctly. Try setting it specifically for this process window:

```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
```

Then try running `.\Build.ps1` again in the **same terminal window**.

### 3. Verify Encoding (If it still fails)

If you recently edited the file in an external editor, it might have saved with an encoding (like UTF-16 with BOM) that PowerShell dislikes.

1. Open `Build.ps1` in **VS Code**.
2. Look at the bottom right status bar; it should say **UTF-8**.
3. If it says anything else, click it, select **"Save with encoding"**, and choose **"UTF-8"**.
4. Save the file and try again.
