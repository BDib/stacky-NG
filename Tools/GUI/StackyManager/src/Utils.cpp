#include "Utils.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <uxtheme.h>
#include <propkey.h>
#include <propvarutil.h>
#include <algorithm>
#include <cwctype>

namespace fs = std::filesystem;

std::wstring GetActiveShortcutPath() {
    PWSTR path = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
    std::wstring result = std::wstring(path) + L"\\.StackyManager\\active";
    CoTaskMemFree(path);
    return result;
}

bool IsSystemDarkMode() {
    DWORD data, dataSize = sizeof(data);
    LSTATUS status = RegGetValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &data, &dataSize);

    return (status == ERROR_SUCCESS && data == 0);
}

void SetPreferredAppMode(int mode) {
    typedef int (WINAPI *pSetPreferredAppMode)(int);
    typedef void (WINAPI *pFlushMenuThemes)();
    auto hUxTheme = LoadLibraryW(L"uxtheme.dll");
    if (hUxTheme) {
        auto SetPreferredAppModeProc = (pSetPreferredAppMode)GetProcAddress(hUxTheme, (LPCSTR)135);
        auto FlushMenuThemesProc = (pFlushMenuThemes)GetProcAddress(hUxTheme, (LPCSTR)136);
        if (SetPreferredAppModeProc) {
            SetPreferredAppModeProc(mode);
        }
        if (FlushMenuThemesProc) {
            FlushMenuThemesProc();
        }
        FreeLibrary(hUxTheme);
    }
}

bool EnsureDataFolders() {
    fs::create_directories(GetActiveShortcutPath());

    PWSTR startMenuPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &startMenuPath))) {
        fs::path shortcutPath = fs::path(startMenuPath) / L"StackyManager.lnk";
        CoTaskMemFree(startMenuPath);

        if (!fs::exists(shortcutPath)) {
            IShellLinkW* psl = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl)))) {
                wchar_t szExePath[MAX_PATH];
                GetModuleFileNameW(NULL, szExePath, MAX_PATH);
                psl->SetPath(szExePath);

                IPropertyStore* pps = nullptr;
                if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARGS(&pps)))) {
                    PROPVARIANT prop;
                    if (SUCCEEDED(InitPropVariantFromString(L"StackyManager", &prop))) {
                        pps->SetValue(PKEY_AppUserModel_ID, prop);
                        pps->Commit();
                        PropVariantClear(&prop);
                    }
                    pps->Release();
                }

                IPersistFile* ppf = nullptr;
                if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARGS(&ppf)))) {
                    ppf->Save(shortcutPath.c_str(), TRUE);
                    ppf->Release();
                }
                psl->Release();
            }
        }
    }
    return true;
}

bool IsStackyShortcut(const std::wstring& lnkPath) {
    IShellLinkW* psl = nullptr;
    bool match = false;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl)))) {
        IPersistFile* ppf = nullptr;
        if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARGS(&ppf)))) {
            if (SUCCEEDED(ppf->Load(lnkPath.c_str(), STGM_READ))) {
                wchar_t szPath[MAX_PATH];
                psl->GetPath(szPath, MAX_PATH, NULL, SLGP_RAWPATH);
                std::wstring targetPath(szPath);
                std::transform(targetPath.begin(), targetPath.end(), targetPath.begin(), ::towlower);

                wchar_t szExePath[MAX_PATH];
                GetModuleFileNameW(NULL, szExePath, MAX_PATH);
                std::wstring currentExe(szExePath);
                std::transform(currentExe.begin(), currentExe.end(), currentExe.begin(), ::towlower);

                if (targetPath.find(L"stackymanager.exe") != std::wstring::npos ||
                    targetPath.find(L"stacky.exe") != std::wstring::npos ||
                    targetPath == currentExe) {
                    match = true;
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    return match;
}
