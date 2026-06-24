#pragma once
#include <windows.h>
#include <string>
#include <vector>

class ShellManager {
public:
	static std::vector<std::wstring> GetShortcutList();
	static void SyncPinnedStackyShortcuts();
	static void EditShortcut(HWND hwnd, const std::wstring& lnkPath);
	static void CreateNewShortcut(HWND hwnd);
	static void DeleteShortcut(const std::wstring& lnkPath);
	static void ShowAboutDialog(HWND hwnd);
    static void OpenShortcutsDirectory();
	static INT_PTR CALLBACK GenericDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
};