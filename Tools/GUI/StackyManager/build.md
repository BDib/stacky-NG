# Building StackyManager

StackyManager is a native Windows C++ application built using CMake and the MSVC compiler.

## Prerequisites

* **Compiler**: Visual Studio 2022+ (with "Desktop development with C++" workload installed).
* **Build System**: CMake 3.20 or higher.
* **Windows SDK**: Version 10.0.28000.0 or higher.

## Dependencies

The project links against the following Windows system libraries:

* `shell32.lib`
* `ole32.lib`
* `uuid.lib`
* `UxTheme.lib`
* `dwmapi.lib`
* `propsys.lib`
* `runtimeobject.lib`

## Build Instructions

### Option 1: Automated Build (Recommended)

We provide a PowerShell script, `Build.ps1`, to automate the configuration, build, and deployment process across multiple architectures.

**Clone the repository**:
```bash
git clone <your-repo-url>
cd StackyManager

```

**Usage:**

```powershell
.\Build.ps1 [options]

```

**Options:**

* `-Help, -h` : Display the help menu.
* `-Clean` : Remove existing `build` and `Bin` directories.
* `-Rebuild` : Perform a clean build followed by a fresh compilation.
* `-Arch <val>` : Specify architecture to build (`x64`, `Win32`, `ARM64`).
* `-Config <val>` : Specify build config (`Debug`, `Release`).

*Example:* To perform a clean rebuild of the Release version for ARM64:

```powershell
.\Build.ps1 -Rebuild -Arch ARM64 -Config Release

```

---

### Option 2: Manual Build

If you prefer to build manually via the command line or IDE:

1. **Clone the repository**:
```bash
git clone <your-repo-url>
cd StackyManager

```


2. **Configure with CMake**:
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

```


3. **Build**:
You can build directly from the command line using CMake:
```bash
cmake --build . --config Release

```


Alternatively, open the generated `StackyManager.sln` file in Visual Studio and build via the UI.
4. **Output**:
Binaries will be placed in the `build/bin/Debug` or `build/bin/Release` directories, depending on your selected configuration.