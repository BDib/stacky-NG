# Setup_Stacky Documentation

## 1. Overview

**Setup_Stacky** is a WPF-based GUI utility designed to streamline the management of **StackyNG** workspaces . It replaces manual command-line configuration with an intuitive Windows interface, ensuring workspaces are structured consistently and launching them is as simple as clicking an icon.

[See Documentation](https://bdib.github.io/stacky-NG/docs/)

### Key Features

* **Directory Orchestration**: Automatically initializes new stack directories and prepares them for application shortcuts.
* **Config Management**: Generates a standard `stacky.json` configuration file, which serves as the "brain" for how StackyNG handles applications within that stack.
* **Shortcut Automation**: Automatically generates a properly configured Windows Desktop shortcut (`.lnk`) that points to your `stacky.exe` and passes the stack directory as an argument.

## 2. Technical Workflow

The application gathers user input through its WPF front-end and executes the following logic:

1. **Validation**: Verifies that the target workspace directory exists (or creates it) and confirms the provided `stacky.exe` path is valid.
2. **JSON Generation**: Creates a base `stacky.json` file inside your workspace. The "Force" option ensures existing configurations are updated to the current schema to prevent corruption.
3. **COM Integration**: Utilizes the `WScript.Shell` COM objectâ€”the same robust engine used by professional installersâ€”to create fully compliant Windows shortcuts.
4. **Parameter Mapping**: Sets the shortcut `TargetPath` to `stacky.exe` and maps the `Arguments` property to your selected folder path, ensuring correct workspace initialization on launch.

---

## 3. Build & Setup Guide

### Prerequisites

* **OS**: Windows 11.
* **SDK**: .NET 10.0 SDK.
* **Compiler**: Visual Studio 2026 (or 2022) with the "Desktop development with C++" and ".NET desktop development" workloads.
* **Dependencies**: Windows Script Host Object Model (COM).

###  Automated Environment Setup

If setting up a fresh build machine, use one of the following methods:

#### Using Chocolatey

```powershell
# Install Build Tools
choco install visualstudio2022buildtools -y
choco install visualstudio2022-workload-manageddesktopbuildtools -y
choco install visualstudio2022-workload-nativedesktop -y
```

#### Using Winget

```powershell
# Install .NET SDK 10.0
winget install Microsoft.DotNet.SDK.10

# Install Build Tools
winget install Microsoft.VisualStudio.2022.BuildTools --override "--passive --add Microsoft.VisualStudio.Workload.ManagedDesktopBuildTools"
```

###  Build Instructions

#### From Visual Studio IDE

1. Open `Setup_Stacky.slnx` in VS2026/2022.
2. Select the **Release** configuration.
3. Build the solution (**Ctrl+Shift+B**).

#### Automated via `Build.ps1`

The project's orchestrator script, `Build.ps1`, automatically builds the GUI utility and deploys the necessary runtime files (`.exe`, `.dll`, `.deps.json`, and `.runtimeconfig.json`) to the `dist\Tools` directory.

* **Standard Build**: Run `.\Build.ps1 -BuildTools` to build all project components.
* **Clean Build**: Run `.\Build.ps1 -Clean` to remove all previous `bin`, `obj`, and `dist` artifacts before rebuilding.
