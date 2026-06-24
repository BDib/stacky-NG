#include "TrayIcon.h"
#include "resource.h"
#include "ShellManager.h"
#include "Utils.h"
#include <shellapi.h>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem; 
extern bool g_isDarkMode;

void CreateTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_APP_ICON));
    wcscpy_s(nid.szTip, _countof(nid.szTip), L"StackyManager");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}


void ShowContextMenu(HWND hwnd) {
    // Note: Syncing on every click might be heavy if there are many files,
    // but ensures the list is always up to date.
    ShellManager::SyncPinnedStackyShortcuts();
    auto shortcuts = ShellManager::GetShortcutList();

    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();

    MENUINFO mi = { sizeof(MENUINFO) };
    mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
    if (g_isDarkMode) {
        static HBRUSH hbrDark = CreateSolidBrush(RGB(45, 45, 45));
        mi.hbrBack = hbrDark;
    } else {
        mi.hbrBack = (HBRUSH)(COLOR_MENU + 1);
    }
    SetMenuInfo(hMenu, &mi);

    for (size_t i = 0; i < shortcuts.size(); ++i) {
        HMENU hSub = CreatePopupMenu();
        SetMenuInfo(hSub, &mi);
        AppendMenuW(hSub, MF_STRING, 100 + (UINT_PTR)i, L"Launch");
        AppendMenuW(hSub, MF_STRING, 200 + (UINT_PTR)i, L"Edit");
        AppendMenuW(hSub, MF_STRING, 300 + (UINT_PTR)i, L"Delete");
        
        fs::path p(shortcuts[i]);
        std::wstring displayName = p.stem().wstring();
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSub, displayName.c_str());
    }
	
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, 5, L"Add New Shortcut...");

    // Settings Submenu
    HMENU hSettings = CreatePopupMenu();
    SetMenuInfo(hSettings, &mi);

    // Active Shortcuts Submenu
    HMENU hActive = CreatePopupMenu();
    SetMenuInfo(hActive, &mi);
    AppendMenuW(hActive, MF_STRING, 10, L"Open Shortcuts Directory");
    AppendMenuW(hActive, MF_STRING, 6, L"Rescan Pinned Shortcuts");

    AppendMenuW(hSettings, MF_POPUP, (UINT_PTR)hActive, L"Shortcut Management");
    AppendMenuW(hSettings, MF_STRING, 7, L"About StackyManager");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSettings, L"Settings");
    AppendMenuW(hMenu, MF_STRING, 2, L"Exit");
    
    SetForegroundWindow(hwnd);
    UINT id = TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                             pt.x, pt.y, 0, hwnd, NULL);
    
    if (id > 0) PostMessage(hwnd, WM_COMMAND, id, 0);
    DestroyMenu(hMenu);
}
