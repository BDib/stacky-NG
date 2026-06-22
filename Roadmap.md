# StackyNG Future Roadmap

This roadmap outlines planned features and potential improvements for future versions of StackyNG.

[See Documentation](https://bdib.github.io/stacky-NG/docs/)

---

## 🚀 Upcoming Features (v0.8.0+)

### 🎨 Visual Enhancements

- **Custom Menu Animations**: Smooth fade-in or slide-out effects for menus.
- **Icon Glow/Effects**: Optional visual feedback when hovering over items.
- **Smart Launch Positioning**: Detect taskbar shortcut location vs cursor position to anchor the menu exactly where the user clicked, replicating the "native" Windows flyout feel.

### ⚙️ Functional Improvements

- **Pinned Items**: Allow certain shortcuts to always stay at the top of the menu, regardless of alphabetical sorting.
- **Recursive Monitoring**: Deep monitoring of subfolders for instant updates.
- **Recent Items Integration**: Optional section at the top of the menu for most recently used shortcuts.
- **Interactive Config UI**: A small companion tool to visually edit `stacky.json` settings.

### 🛠 Technical Debt & Optimization

- **Incremental Cache Updates**: Instead of a full rebuild when a change is detected, only update the changed item.
- **Improved Logging**: Implement log rotation and configurable verbosity levels.
- **Comprehensive JSON Support**: Transition from the current minimal parser to a standard, lightweight JSON library for more complex configuration.

---

## 💡 Long-term Vision (v1.0.0+)

### 🌐 Cross-Platform (Partial)

- While the core is Win32-based, explore a companion configuration GUI built in a cross-platform framework (e.g., Qt or Dear ImGui) to make `stacky.json` editing easier.

### 📦 Portable Stacks

- A feature to "package" a stack (shortcuts + config + icons) into a single portable directory for easy syncing across multiple Windows machines.

### ⌨️ Hotkey Support

- Allow triggering specific stacks via system-wide global hotkeys.

---

## 🤝 Community Contributions

We welcome ideas and pull requests! If you have a feature suggestion, please open an issue or start a discussion.
