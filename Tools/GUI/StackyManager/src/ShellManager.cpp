#include "ShellManager.h"
#include "resource.h"
#include "Utils.h"
#include "NotificationManager.h"
#include <uxtheme.h>
#include <windows.ui.shell.h>
#include <windows.foundation.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dwmapi.h>
#include <commdlg.h>
#include <shlobj.h>
#include <propkey.h>
#include <propvarutil.h>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

struct ShortcutData { std::wstring name; std::wstring target; std::wstring args; };

extern bool g_isDarkMode;

bool ValidateInputs(const ShortcutData& data) {
    if (data.target.empty()) return false;
    DWORD attrTarget = GetFileAttributesW(data.target.c_str());
    if (attrTarget == INVALID_FILE_ATTRIBUTES) return false;
    return true;
}

// Modern Folder Picker using IFileOpenDialog
HRESULT PickFolder(HWND hwndOwner, std::wstring& outPath) {
    IFileOpenDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        if (SUCCEEDED(pfd->Show(hwndOwner))) {
            IShellItem* psi = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                LPWSTR pszPath = nullptr;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    outPath = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    return hr;
}

INT_PTR CALLBACK EditDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
    static ShortcutData* data;
    switch (msg) {
        case WM_INITDIALOG:
        {
            if (g_isDarkMode) {
                BOOL useImmersiveDarkMode = TRUE;
                DwmSetWindowAttribute(hDlg, 20, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
                EnumChildWindows(hDlg, [](HWND hwnd, LPARAM lp) -> BOOL {
                    SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);
                    return TRUE;
                }, 0);
            }
            data = (ShortcutData*)lp;
            SetDlgItemTextW(hDlg, IDC_EDIT_NAME, data->name.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_TARGET, data->target.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_ARGS, data->args.c_str());
            return TRUE;
        }
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN:
        {
            if (g_isDarkMode) {
                HDC hdc = (HDC)wp;
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkColor(hdc, RGB(32, 32, 32));
                static HBRUSH hbrDark = CreateSolidBrush(RGB(32, 32, 32));
                return (INT_PTR)hbrDark;
            }
            break;
        }
        case WM_COMMAND:
            if (wp == IDC_BTN_BROWSE_EXE) {
                OPENFILENAMEW ofn = { sizeof(ofn) };
                wchar_t szFile[MAX_PATH] = { 0 };
                ofn.hwndOwner = hDlg;
                ofn.lpstrFilter = L"Executable (*.exe)\0*.exe\0All\0*.*\0";
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST;
                if (GetOpenFileNameW(&ofn)) {
                    SetDlgItemTextW(hDlg, IDC_EDIT_TARGET, szFile);
                }
            } else if (wp == IDC_BTN_BROWSE_DIR) {
                std::wstring pickedPath;
                if (SUCCEEDED(PickFolder(hDlg, pickedPath))) {
                    if (!pickedPath.empty()) {
                        SetDlgItemTextW(hDlg, IDC_EDIT_ARGS, pickedPath.c_str());
                    }
                }
            } else if (wp == IDOK) {
                wchar_t bufName[MAX_PATH], bufTarget[MAX_PATH], bufArgs[MAX_PATH];
                GetDlgItemTextW(hDlg, IDC_EDIT_NAME, bufName, MAX_PATH);
                GetDlgItemTextW(hDlg, IDC_EDIT_TARGET, bufTarget, MAX_PATH);
                GetDlgItemTextW(hDlg, IDC_EDIT_ARGS, bufArgs, MAX_PATH);
                
                ShortcutData temp = { bufName, bufTarget, bufArgs };
                if (ValidateInputs(temp)) {
                    *data = temp;
                    EndDialog(hDlg, IDOK);
                } else {
                    NotificationManager::ShowNotification(L"Error", L"Invalid target path.", hDlg);
                }
            } else if (wp == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
            }
            return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK ShellManager::GenericDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG:
            if (g_isDarkMode) {
                BOOL useImmersiveDarkMode = TRUE;
                DwmSetWindowAttribute(hDlg, 20, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
                EnumChildWindows(hDlg, [](HWND hwnd, LPARAM lp) -> BOOL {
                    SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);
                    return TRUE;
                }, 0);
            }
            return TRUE;
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
            if (g_isDarkMode) {
                HDC hdc = (HDC)wp;
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkColor(hdc, RGB(32, 32, 32));
                static HBRUSH hbrDark = CreateSolidBrush(RGB(32, 32, 32));
                return (INT_PTR)hbrDark;
            }
            break;
        case WM_COMMAND:
            if (wp == IDOK || wp == IDCANCEL) EndDialog(hDlg, wp);
            return TRUE;
    }
    return FALSE;
}

void ShellManager::ShowAboutDialog(HWND hwnd) {
    DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_ABOUT), hwnd, GenericDlgProc, 0);
}

void ShellManager::CreateNewShortcut(HWND hwnd) {
    ShortcutData data = { L"", L"", L"" };
    if (DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_EDIT_SHORTCUT), hwnd, EditDlgProc, (LPARAM)&data) == IDOK) {
        IShellLinkW* psl = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl)))) {
            psl->SetPath(data.target.c_str());
            psl->SetArguments(data.args.c_str());
            
            std::wstring finalName = data.name;
            if (finalName.empty()) {
                finalName = fs::path(data.target).stem().wstring();
                if (finalName.empty()) finalName = L"NewShortcut";
            }

            fs::path dir = GetActiveShortcutPath();
            fs::path newPath = dir / (finalName + L".lnk");
            int count = 1;
            while(fs::exists(newPath)) {
                newPath = dir / (finalName + L"_" + std::to_wstring(count++) + L".lnk");
            }
            
            IPersistFile* ppf = nullptr;
            if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARGS(&ppf)))) {
                ppf->Save(newPath.c_str(), TRUE);
                ppf->Release();
                NotificationManager::ShowNotification(L"StackyManager", L"Shortcut created.", hwnd);
            }
            psl->Release();
        }
    }
}

void ShellManager::DeleteShortcut(const std::wstring& lnkPath) {
    std::error_code ec;
    if (fs::exists(lnkPath, ec)) {
        if (fs::remove(lnkPath, ec)) {
            // Deleted successfully
        } else {
             std::wstring errorMsg = L"Delete failed: " + std::wstring(ec.message().begin(), ec.message().end());
             NotificationManager::ShowNotification(L"Error", errorMsg, NULL);
        }
    }
}

void ShellManager::EditShortcut(HWND hwnd, const std::wstring& lnkPath) {
    if (!fs::exists(lnkPath)) {
        NotificationManager::ShowNotification(L"StackyManager", L"Shortcut not found.", hwnd);
        return;
    }

    IShellLinkW* psl = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl)))) {
        IPersistFile* ppf = nullptr;
        if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARGS(&ppf)))) {
            if (SUCCEEDED(ppf->Load(lnkPath.c_str(), STGM_READWRITE))) {
                wchar_t path[MAX_PATH], args[MAX_PATH];
                psl->GetPath(path, MAX_PATH, NULL, SLGP_RAWPATH);
                psl->GetArguments(args, MAX_PATH);

                ShortcutData data = { fs::path(lnkPath).stem().wstring(), path, args };
                if (DialogBoxParamW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_EDIT_SHORTCUT), hwnd, EditDlgProc, (LPARAM)&data) == IDOK) {
                    psl->SetPath(data.target.c_str());
                    psl->SetArguments(data.args.c_str());
                    std::wstring newLnkName = data.name + L".lnk";
                    fs::path newPath = fs::path(lnkPath).parent_path() / newLnkName;

                    if (newPath != lnkPath) {
                        ppf->Save(newPath.c_str(), TRUE);
                        fs::remove(lnkPath);
                    } else {
                        ppf->Save(lnkPath.c_str(), TRUE);
                    }
                    NotificationManager::ShowNotification(L"StackyManager", L"Shortcut updated.", hwnd);
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
}

std::vector<std::wstring> ShellManager::GetShortcutList() {
    std::vector<std::wstring> shortcuts;
    std::wstring path = GetActiveShortcutPath();
    if (fs::exists(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == L".lnk") {
                shortcuts.push_back(entry.path().wstring());
            }
        }
    }
    std::sort(shortcuts.begin(), shortcuts.end());
    return shortcuts;
}

void ShellManager::SyncPinnedStackyShortcuts() {
    std::wstring activePath = GetActiveShortcutPath();
    if (!fs::exists(activePath)) fs::create_directories(activePath);

    PWSTR pinnedPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &pinnedPath))) {
        fs::path taskbarDir = fs::path(pinnedPath) / L"Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar";
        CoTaskMemFree(pinnedPath);
        if (fs::exists(taskbarDir)) {
            for (const auto& entry : fs::directory_iterator(taskbarDir)) {
                if (entry.path().extension() == L".lnk") {
                    if (IsStackyShortcut(entry.path().wstring())) {
                        fs::path dest = fs::path(activePath) / entry.path().filename();
                        if (!fs::exists(dest)) {
                            fs::copy_file(entry.path(), dest);
                        }
                    }
                }
            }
        }
    }
}

void ShellManager::OpenShortcutsDirectory() {
    std::wstring path = GetActiveShortcutPath();
    ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
