# StackyManager

A lightweight, system-tray-based shortcut manager for Windows. 
StackyManager allows users to quickly launch, edit, and organize shortcuts directly from the system tray, with support for synchronizing your pinned taskbar items.

## Features

-   **Tray-based management**: Launch, edit, and delete shortcuts without cluttering your desktop.
-   **Settings Menu**: Organized access to application options.
-   **Direct Management**: Shortcuts are managed directly in an `active` folder.
-   **Rescan Taskbar**: Automatically pull shortcuts from your Windows Taskbar pinned items that point back to Stacky.
-   **Theme Support**: Automatically follows system theme (Light/Dark mode) for menus and dialogs.
-   **Modern Notifications**: Visual feedback via WinRT Toast notifications with a reliable fallback to tray balloon tips.

## How to Use

1.  **Run `StackyManager.exe`**: The application will appear in your system tray.
2.  **Add/Edit Shortcuts**: Right-click the tray icon to add new shortcuts or edit existing ones.
3.  **Launch**: Simply click a shortcut name in the tray menu to launch it.
4.  **Manual Pinning**: To pin a shortcut to your Windows Taskbar, go to `Settings > Shortcut Management > Open Shortcuts Directory`, then right-click the desired shortcut and select "Pin to taskbar". See [tutorial.md](tutorial.md) for more details.

## Step-by-Step Guide to manual pinning

1.  **Right-click** the StackyManager tray icon.
2.  Navigate to **Settings** > **Shortcut Management**.
3.  Select **Open Shortcuts Directory**. This will open File Explorer at `%AppData%\.StackyManager\active`.
4.  In the Explorer window, you will see your shortcuts as `.lnk` files.
5.  **Right-click** the shortcut you want to pin.
6.  Select **Show more options** (if on Windows 11).
7.  Click **Pin to taskbar**.

## Tip: Drag and Drop

You can also drag any shortcut directly from the `active` directory onto your Taskbar to pin it!

---

## Feature Status

| Feature | Status | Notes |
| :--- | :--- | :--- |
| **Shortcut Launching** | ✅ Working | Reliable `ShellExecute` implementation. |
| **Theme (Dark/Light)** | ✅ Working | Native Windows theme following. |
| **Notifications** | ✅ Working | Modern WinRT Toasts with Tray Fallback. |
| **Taskbar Sync** | ✅ Working | Syncs existing taskbar shortcuts to Stacky. |
| **Manual Pinning** | ✅ Documented | Programmatic shortcut pinning is restricted by Windows; manual pinning is the recommended way. |

## License

StackyManager is licensed under the Apache License, Version 2.0. See the `LICENSE` and `NOTICE` files for details.
