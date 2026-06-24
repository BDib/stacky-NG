### Documentation: `setup_stacky.ps1`

#### Overview

`setup_stacky.ps1` is a deployment utility that automates the setup of StackyNG workspaces. It prepares a target directory for use by creating a default configuration and providing a desktop shortcut to launch StackyNG directly into that workspace.

#### Requirements

* **OS**: Windows 10/11.
* **Execution Policy**: Must have permissions to run PowerShell scripts (`Set-ExecutionPolicy RemoteSigned`).

#### Parameters

| Parameter | Required | Description |
| --- | --- | --- |
| `-StackDir` | Yes | The full path to the folder containing your application `.lnk` files. |
| `-StackyExe` | Yes | The full path to your compiled `stacky.exe`. |
| `-ShortcutName` | No | Custom name for the desktop shortcut (default: `Stacky_[FolderName]`). |
| `-Force` | No | Overwrites existing `stacky.json` and `.lnk` files. |
| `-Verbose` | No | Displays detailed execution progress. |

#### How It Works

1. **Validation**: Ensures the provided `StackDir` exists (creates it if missing) and verifies that `stacky.exe` is a valid file path.
2. **Configuration**: Creates a `stacky.json` inside the `StackDir` if it does not exist. This file provides the schema for customizing how StackyNG manages the shortcuts within that folder.
3. **Shortcut Creation**: Creates a shortcut on your Desktop. It automatically maps the `TargetPath` to your `stacky.exe` and sets the `Arguments` to the `StackDir` path, ensuring that when you click the icon, StackyNG opens exactly the intended stack.

#### Usage Example

To set up a new stack in `C:\Stacks\WorkApps` using your newly built `stacky.exe`:

```powershell
.\setup_stacky.ps1 -StackDir "C:\Stacks\WorkApps" -StackyExe "C:\Project\dist\x64\Release\stacky.exe" -ShortcutName "Work_Apps" -Force -Verbose

```