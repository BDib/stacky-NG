#Setup_Stacky

##Description

The **Setup_Stacky** GUI application is a specialized automation tool designed to streamline the management of **StackyNG** workspaces. It replaces manual command-line configuration with an intuitive Windows interface, ensuring that workspaces are structured consistently and that launching them is as simple as clicking a desktop/taskbar icon.

### What it Does

* **Directory Orchestration**: Automatically initializes new stack directories, ensuring they are ready to hold your application shortcuts.
* **Config Management**: Generates a standard `stacky.json` configuration file, which serves as the "brain" for how StackyNG handles the applications within that stack.
* **Shortcut Automation**: Automatically generates a properly configured Windows Desktop shortcut (`.lnk`) that points to your `stacky.exe` and passes the stack directory as an argument.

---

### How it Works

The application uses a WPF (Windows Presentation Foundation) front-end to gather user inputs and executes the following logic:

1. **Validation**: It verifies that the target workspace directory exists (or creates it) and confirms that the provided `stacky.exe` path is valid.
2. **JSON Generation**: It creates a base `stacky.json` file inside your workspace. If you choose the "Force" option, it ensures any existing configuration is updated to the default schema, preventing accidental corruption.
3. **COM Integration**: It utilizes the `WScript.Shell` COM object—the same robust Windows scripting engine used by professional deployment installers—to create the desktop shortcut.
4. **Parameter Mapping**: It sets the shortcut’s `TargetPath` to your `stacky.exe` and maps the `Arguments` property to your selected folder path, ensuring that launching the icon triggers StackyNG to open that specific workspace.

### Workflow Overview

* **Input**: You provide the path to your `stacky.exe`, your desired folder path, and a custom shortcut name.
* **Processing**: The C# back-end validates these paths, creates the necessary files, and leverages the Windows Script Host to ensure the shortcut is fully compliant with Windows' standards.
* **Output**: A professional-grade desktop shortcut appears on your screen, ready to launch your StackyNG environment immediately, from either the Desktop, or by pinning it in your taskbar and launching it.

### 1. Command Line Build

Open your terminal in the project root (where the `.sln` or `.csproj` file resides) and run:

```powershell
# Clean previous artifacts
dotnet clean

# Build the project in Release mode
dotnet build -c Release

```

If you want to create a standalone executable (no dependencies required on the target machine), use this command:

```powershell
# Creates a single, portable executable
dotnet publish -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true

```

---

### 2. Documentation for `Setup_Stacky`

This guide explains how to compile and distribute your new GUI setup utility.

# Setup_Stacky Build Guide

The Setup_Stacky tool is a WPF-based GUI utility for managing StackyNG workspace folders and generating desktop shortcuts.

## 🏗 Build Requirements

* **OS**: Windows 11.
* **SDK**: .NET 10.0 SDK.
* **Compiler**: Visual Studio 2026 (or 2022) with "Desktop development with C++" workload.
* **Dependencies**: Windows Script Host Object Model (COM).

## 🛠 Automated Setup (Command Line)

If you are setting up a clean build machine, run these commands to install the necessary components:

### Using Chocolatey

```powershell
# Install Build Tools
choco install visualstudio2022buildtools -y
choco install visualstudio2022-workload-manageddesktopbuildtools -y

# Install Desktop Development Support
choco install visualstudio2022-workload-nativedesktop -y

```

### Using Winget

```powershell
# Install .NET SDK 10.0
winget install Microsoft.DotNet.SDK.10

# Install Build Tools
winget install Microsoft.VisualStudio.2022.BuildTools --override "--passive --add Microsoft.VisualStudio.Workload.ManagedDesktopBuildTools"

```

---

## 🚀 Build Instructions

### From Command Line

The easiest way to build the project without opening the IDE is via the `dotnet` CLI:

1. **Build**: `dotnet build -c Release`
2. **Publish (Standalone)**:
```powershell
dotnet publish -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true

```


*The resulting executable will be found in: `bin\Release\net10.0-windows\win-x64\publish\*`

### From Visual Studio IDE

1. Open `Setup_Stacky.slnx` in VS2026.
2. Select **Release** configuration.
3. Build the solution (**Ctrl+Shift+B**).
