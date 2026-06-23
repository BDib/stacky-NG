# StackyNG v0.7.0.1 - Feature Overview & System Description

**StackyNG** is a modernized, high-performance Win32 utility designed to declutter the Windows taskbar by grouping shortcuts into organized, hierarchical menus ("stacks"). It serves as a successor to the original Stacky project, rewritten for speed, stability, and compatibility with modern Windows 10/11 environments.
[See Documentation](https://bdib.github.io/stacky-NG/docs/)
---

## 🚀 Key Features Implemented

### 1. High-Performance Architecture

- **Asynchronous Scanning**: Unlike legacy tools that freeze the UI while scanning folders, StackyNG uses a background worker thread. You see a "Loading..." menu immediately, which dynamically refreshes once the data is ready.
- **Memory-Mapped Cache**: Shortcut data and icons are stored in `!stacky.cache` using Win32 Memory-Mapped Files (`CreateFileMapping`). This allows near-instant loading on subsequent launches by mapping the file directly into the process memory space.
- **Structural Validation**: On launch, StackyNG performs a lightning-fast structural scan (names and timestamps only) and compares it against the cache. It only rebuilds expensive resources (like icons) if a change is detected.

### 2. Modern Visuals & High DPI

- **WIC (Windows Imaging Component)**: Uses the modern WIC API for high-quality, linear scaling of icons, ensuring they look sharp regardless of the source format.
- **High-DPI Awareness**: Utilizes `SHGetImageList` to extract jumbo/large icons from the system, preventing the "blurry icon" syndrome common in older Win32 apps.
- **Dark Mode Support**: Deep integration with the Windows 10/11 "Immersive Dark Mode." The application detects your system theme and applies dark-themed menus (`DarkMode_Explorer`) and tooltips automatically.
- **Dynamic Tooltips**: Hovering over any menu item displays a tooltip showing the target path or the shortcut's metadata description.

### 3. Advanced Customization (`stacky.json`)

Every stack folder can contain a `stacky.json` file to override default behavior:

- **Custom Naming**: Change how a shortcut appears in the menu without renaming the file on disk.
- **Icon Overrides**: Support for external `.png` or `.ico` icons (including transparency).
- **Argument Injection**: Pass custom command-line arguments to applications when launched from the stack.
- **Administrative Elevation**: Flag specific shortcuts to always "Run as Administrator."
- **Theme Override**: Force "dark", "light", or "auto" themes per stack.

### 4. Robustness & System Integration

- **Real-Time Monitoring**: Uses `ReadDirectoryChangesW` to watch your stack folder. If you add or remove a file while the menu is open, StackyNG detects the change and triggers an async refresh.
- **Circular Link Protection**: Automatically detects and ignores circular directory junctions or symbolic links, preventing infinite recursive menu loops.
- **Single Instance Signaling**: If you click a StackyNG shortcut while a menu is already open, the new process signals the existing one to refresh and reposition, then exits immediately.
- **Clean UI Logic**: Implemented as a "Tool Window" to stay out of the Alt-Tab list and Taskbar, providing a seamless "native" feel.

---

## 🛠 Project Structure (Modular v0.7.0.1)

The code has been fully refactored into modular components for easier maintenance:

- `App`: Manages the Win32 window, message loop, and menu display.
- `CacheManager`: Handles filesystem scanning, structural comparison, and cache serialization.
- `Bmp`: Logic for icon extraction, WIC processing, and bitmap management.
- `Config`: A lightweight, fast parser for hierarchical `stacky.json` files.
- `Util`: Shared Win32 helpers, logging, and memory-mapping logic.
- `Logger`: Thread-safe audit trail for troubleshooting background operations.

---

## 📦 Build System

StackyNG supports multiple professional build environments:

- **CMake**: Modern cross-platform foundation (min 3.15).
- **Visual Studio 2022**: Full `.sln` and `.vcxproj` support.
- **NMake**: Supplemental Makefile for command-line power users.
- **Batch**: Simple `build.bat` for quick one-click compilation.
