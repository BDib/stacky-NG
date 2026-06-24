#pragma once
#include <string>
#include <windows.h>

#define ID_TRAY_ICON 5000

std::wstring GetActiveShortcutPath();
bool IsSystemDarkMode();
void SetPreferredAppMode(int mode);
bool EnsureDataFolders();
bool IsStackyShortcut(const std::wstring& lnkPath);
