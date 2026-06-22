# StackyNG v0.7.0

Simple icon stacks solution for Windows 11

---

StackyNG is a small, fast program for Windows that displays the contents of a directory as a list of clickable, labeled icons. It's designed to replace multiple taskbar shortcuts with a single, organized menu.

---

## Documentation
You can find our full documentation, including roadmaps and guides, at the link below:

[https://bdib.github.io/stacky-NG/docs/](https://bdib.github.io/stacky-NG/docs/)

---

## New in v0.7.0 (StackyNG)

- **Modular Refactor**: Clean, maintainable codebase split into logic-specific modules.
- **Improved Stability**: Fixed menu "escaping" and UI ghosting issues.
- **Expanded Build Support**: Native support for VS2022, CMake, Ninja , NMake, and MSBuild.
- **Structured Configuration**: Enhanced `stacky.json` for advanced shortcut overrides.
- **Custom Icon Overrides**: Support for `.png`, `.ico`, and other formats with transparency.
- **Dark Mode Support**: Automatically follows system theme or allows manual override.
- **Dynamic Tooltips**: Hover over items to see their full path or description.
- **Argument & Admin Support**: Launch shortcuts with custom command-line arguments or as Administrator.
- **Asynchronous Scanning**: UI remains responsive while the cache is being built.
- **Memory-Mapped Cache**: High-performance Win32 cache I/O.
- **High DPI Support**: Optimized icon extraction and scaling.

---

## Architecture Overview

StackyNG follows an optimized scan-cache-display cycle:

1. **Command Line**: Receives a folder path as its first argument.
2. **Instance Signaling**: If another instance is running, signals it to show the menu and exits.
3. **Asynchronous Scan**: Recursively scans the target folder for `.lnk` files on a background thread.
4. **Memory-Mapped Cache**:
   - Loads the icon and tree structure using memory mapping for maximum performance.
   - Validates cache timestamp against filesystem modification times.
5. **Modern Icon Extraction**: Uses Windows Shell `IImageList` and WIC to extract and scale icons correctly for High DPI.
6. **Menu Construction**: Dynamically builds a hierarchical `HMENU`.
7. **Interaction**: Displays the menu at the cursor position and launches the target with `ShellExecute`.

---

## Customization (`stacky.json`)

StackyNG supports advanced customization via a `stacky.json` file. You can override names, icons, and launch arguments for individual items.

Example:

```json
"MyGame": {
    "name": "Play Modern Warfare",
    "icon": "icons/mw.png",
    "args": "--developer-mode",
    "admin": true
}
```

---

## How to Use It

1. **Create a Folder**: Place shortcuts to your programs inside it. Subdirectories can be used for organization.  

2. **Create a Shortcut**: Make a Windows shortcut to `stacky.exe`.  

3. **Set the Target**: Edit the shortcut’s target to include your folder path:
   
   ```dos
   "C:\Path\To\stacky.exe" "C:\Path\To\Your\StackFolder"
   ```

4. **Pin to Taskbar**: Drag the shortcut to the taskbar.  

---

## Compilation

See [Build](https://bdib.github.io/stacky-NG/docs/build/) for detailed build instructions using CMake, Visual Studio, or the command line.

---

## Build Requirements

- **Compiler**: C++17 compatible (Visual Studio 2022 or later).  
- **Architectures**: x64, x86, ARM64.  
- **SDKs**: Windows SDK (Win32 + WIC + Common Controls APIs).
- **Libraries**: `user32`, `gdi32`, `shell32`, `ole32`, `comctl32`, `windowscodecs`.

---

## License

StackyNG is open source and released under the **Apache License 2.0**.
Original project by Pawel Turlejski. Modernized and maintained by B Dib.
