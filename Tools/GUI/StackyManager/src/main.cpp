#include <windows.h>
#include <roapi.h>
#include <shobjidl.h>
#include "resource.h"
#include "ShellManager.h"
#include "TrayIcon.h"
#include "NotificationManager.h"
#include "Utils.h"

bool g_isDarkMode = true;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TRAYICON:
            if (lp == WM_RBUTTONUP) ShowContextMenu(hwnd);
            break;

        case WM_SETTINGCHANGE:
            if (lp && lstrcmpW((LPCWSTR)lp, L"ImmersiveColorSet") == 0) {
                g_isDarkMode = IsSystemDarkMode();
                SetPreferredAppMode(g_isDarkMode ? 2 : 1);
            }
            break;

        case WM_COMMAND:
            if (wp == 10) {
                ShellManager::OpenShortcutsDirectory();
			} else if (wp == 7) {
				ShellManager::ShowAboutDialog(hwnd);
			} else if (wp == 6) {
				ShellManager::SyncPinnedStackyShortcuts();
				NotificationManager::ShowNotification(L"StackyManager", L"Rescan complete.", hwnd);
            } else if (wp == 5) {
                ShellManager::CreateNewShortcut(hwnd);
            } else if (wp == 2) { 
                PostQuitMessage(0);
            } else if (wp >= 300) {
                int index = (int)wp - 300;
                auto list = ShellManager::GetShortcutList();
                if (index >= 0 && index < list.size()) {
                    if (DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_CONFIRM_DELETE), hwnd, ShellManager::GenericDlgProc, 0) == IDOK) {
                        ShellManager::DeleteShortcut(list[index]);
                        NotificationManager::ShowNotification(L"Success", L"Shortcut deleted.", hwnd);
                    }
                }
            } else if (wp >= 200) { 
                int index = (int)wp - 200;
                auto list = ShellManager::GetShortcutList();
                if (index < list.size()) ShellManager::EditShortcut(hwnd, list[index]);
            } else if (wp >= 100) { 
                int index = (int)wp - 100;
                auto list = ShellManager::GetShortcutList();
                if (index < list.size()) ShellExecuteW(NULL, L"open", list[index].c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"StackyManager_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    SetCurrentProcessExplicitAppUserModelID(L"StackyManager");

    // Use SINGLETHREADED (STA) for UI thread to ensure shell dialogs (IFileOpenDialog) work correctly.
    RoInitialize(RO_INIT_SINGLETHREADED);
    g_isDarkMode = IsSystemDarkMode();
    SetPreferredAppMode(g_isDarkMode ? 2 : 1);

    EnsureDataFolders();
    ShellManager::SyncPinnedStackyShortcuts();

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StackyMgrClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(L"StackyMgrClass", L"", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    CreateTrayIcon(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RoUninitialize();
    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return (int)msg.wParam;
}