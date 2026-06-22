# StackyNG Tutorial

This tutorial provides a comprehensive guide on how to get the most out of **StackyNG**, the modern icon stack solution for the Windows taskbar.

 [See Documentation](https://bdib.github.io/stacky-NG/docs/)

---

## 1. Why StackyNG?

### The Problem

Windows taskbar shortcuts are great, but they quickly become cluttered. If you have 20 games or 15 development tools, pinning them all individually takes up precious screen real estate.

### The StackyNG Solution

StackyNG allows you to group related shortcuts into a single "stack." When you click the stack, a clean, high-performance menu pops up with all your icons, beautifully rendered and ready to launch.

### Comparison with Other Tools

| Feature | Original Stacky | 7stacks | **StackyNG** |
| :--- | :--- | :--- | :--- |
| **Performance** | Basic | Moderate | **High (Async + Memory-Mapped)** |
| **High DPI** | Poor | Poor | **Excellent (WIC + SHGetImageList)** |
| **Hierarchical** | No | Limited | **Full Subfolder Support** |
| **Monitoring** | Full Rescan | No | **Real-time (Incremental)** |
| **Architecture** | 32-bit | Legacy | **Native x64/ARM64** |
| **Customization** | None | GUI-based | **Simple `stacky.json` overrides** |

---

## 2. Basic Setup

### Step 1: Create your Stack Folder

Create a folder anywhere on your computer (e.g., `C:\Stacks\Games`). This folder will hold all the shortcuts you want to see in your menu.

### Step 2: Add Shortcuts

Copy and paste Windows shortcuts (`.lnk` files) into this folder.

- You can right-click any `.exe` and select **Create shortcut**, then move that shortcut into your Stack folder.

### Step 3: Create the StackyNG Launcher

1. Right-click `stacky.exe` and select **Create shortcut**.
2. Right-click the new shortcut and select **Properties**.
3. In the **Target** field, add the path to your stack folder after the executable path:

   ```cmd
   "C:\Path\To\stacky.exe" "C:\Stacks\Games"
   ```

4. Click **OK** and rename the shortcut to something like "My Games".

### Step 4: Pin to Taskbar

Drag your new launcher shortcut to the Windows taskbar.

---

## 3. Advanced Organization

### Hierarchical Menus (Subfolders)

StackyNG automatically turns subdirectories into submenus.

- `C:\Stacks\Work\Office\` -> Becomes a submenu "Office >"
- `C:\Stacks\Work\Design\` -> Becomes a submenu "Design >"

Folders will only appear in the menu if they contain at least one `.lnk` file (or a subfolder that does).

### Using Symlinks and Junctions

StackyNG fully supports NTFS symbolic links and junctions. This is useful for including folders from different drives without moving them.

- **Example**: To include your Steam games folder in a stack:

  ```cmd
  mklink /J "C:\Stacks\Games\Steam" "D:\SteamLibrary\steamapps\common"
  ```

StackyNG has built-in protection to detect and ignore circular junctions, preventing infinite menu loops.

---

## 4. Customization (`stacky.json`)

StackyNG supports advanced customization via a `stacky.json` file placed in your stack folder.

### Global Settings

You can force the theme of the menu:

```json
"theme": "dark"  // Options: "auto", "dark", "light"
```

### Item Overrides

You can customize individual shortcuts by using their file name (without extension) as the key:

```json
"MyGame": {
    "name": "Play Modern Warfare",
    "icon": "icons/mw.png",
    "args": "--developer-mode",
    "admin": true
}
"Browser": {
    "name": "Web Browser"
}
```

- **`name`**: The text displayed in the menu.
- **`icon`**: Path to a `.png`, `.ico`, or `.jpg` file. Supports transparency.
- **`args`**: Custom command-line arguments passed to the application.
- **`admin`**: If set to `true`, the application will launch with Administrator privileges (triggering a UAC prompt).

StackyNG will automatically detect this file and apply the changes.

### Windows 11 Examples

On Windows 11, many system apps are stored in protected or non-standard locations. You can use `stacky.json` to make them look and behave exactly how you want.

#### Root Folder Example (`C:\Stacks\Main\stacky.json`)

```json
"theme": "auto"

"Notepad": {
    "name": "Text Editor",
    "icon": "C:/Windows/System32/notepad.exe"
}
"Calculator": {
    "name": "Math",
    "icon": "C:/Windows/System32/calc.exe"
}
"RegistryEditor": {
    "name": "Registry Editor (Admin)",
    "args": "",
    "admin": true
}
```

#### Subfolder Example (`C:\Stacks\Main\System\stacky.json`)

You can have different settings for each subfolder. Subfolders will only use the `stacky.json` located within them.

```json
"CMD": {
    "name": "Terminal",
    "args": "/k echo Welcome to StackyNG!",
    "admin": true
}
"TaskMgr": {
    "name": "Activity Monitor"
}
```

---

## 5. Troubleshooting & Logs

If an icon isn't appearing correctly or a shortcut isn't launching, check the log file:

- Location: Look for `!stacky.log` in your stack folder.
- Cache: StackyNG uses a hidden `!stacky.cache` file to speed up loading. If things look outdated, StackyNG will usually auto-refresh, but you can always delete this file to force a deep rescan.
