#include "util.h"
#include <Shlobj.h>

bool MappedFile::open(const fs::path& path) {
    close();
    hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER liSize;
    if (!GetFileSizeEx(hFile, &liSize)) {
        close();
        return false;
    }
    size = static_cast<size_t>(liSize.QuadPart);
    if (size == 0) {
        close();
        return false;
    }

    hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        close();
        return false;
    }

    pData = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pData) {
        close();
        return false;
    }

    return true;
}

void MappedFile::close() {
    if (pData) UnmapViewOfFile(pData);
    if (hMapping) CloseHandle(hMapping);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    pData = nullptr;
    hMapping = nullptr;
    hFile = INVALID_HANDLE_VALUE;
    size = 0;
}

String StackyUtil::get_description(const fs::path& path) {
    String desc = L"";
    IShellLinkW* psl = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
        IPersistFile* ppf = nullptr;
        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
            if (SUCCEEDED(ppf->Load(path.c_str(), STGM_READ))) {
                Char buf[MAX_PATH];
                if (SUCCEEDED(psl->GetDescription(buf, MAX_PATH))) {
                    desc = buf;
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    return desc;
}

bool StackyUtil::is_dark_mode() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        DWORD size = sizeof(DWORD);
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
        return value == 0;
    }
    return false;
}

typedef enum PreferredAppMode {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
} PreferredAppMode;

typedef PreferredAppMode(WINAPI* pSetPreferredAppMode)(PreferredAppMode appMode);
typedef void(WINAPI* pFlushMenuThemes)();

void StackyUtil::set_dark_mode(HWND hwnd, bool dark) {
    HMODULE hUxtheme = GetModuleHandleW(L"uxtheme.dll");
    if (hUxtheme) {
        // Ordinal 135 is SetPreferredAppMode
        pSetPreferredAppMode fnSetPreferredAppMode = (pSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        // Ordinal 136 is FlushMenuThemes
        pFlushMenuThemes fnFlushMenuThemes = (pFlushMenuThemes)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136));

        if (fnSetPreferredAppMode) {
            fnSetPreferredAppMode(dark ? ForceDark : ForceLight);
        }
        if (fnFlushMenuThemes) {
            fnFlushMenuThemes();
        }
    }

    BOOL value = dark ? TRUE : FALSE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE
    // Use 20 for Windows 11/Modern Win10, 19 for older Win10
    DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value));
    DwmSetWindowAttribute(hwnd, 19, &value, sizeof(value));

    SetWindowTheme(hwnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
}

String StackyUtil::trim(const String& target, const String& trim_chars) {
    size_t first = target.find_first_not_of(trim_chars);
    if (String::npos == first) return L"";
    size_t last = target.find_last_not_of(trim_chars);
    return target.substr(first, (last - first + 1));
}

void StackyUtil::signal_other_stackies() {
    HWND hwnd = FindWindowW(STACKY_WINDOW_NAME, STACKY_WINDOW_NAME);
    if (hwnd) {
        PostMessage(hwnd, WM_COMMAND, WM_OPEN_TARGET_FOLDER, 1);
        ExitProcess(0);
    }
}

HANDLE StackyUtil::create_mutex() {
    // Unique name for the mutex based on the window name
    return CreateMutexW(nullptr, FALSE, (L"Local\\" + String(STACKY_WINDOW_NAME)).c_str());
}

Time StackyUtil::get_modified(const fs::path& path) {
    try {
        if (!fs::exists(path)) return 0;
        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        return std::chrono::system_clock::to_time_t(sctp);
    } catch (...) {
        return 0;
    }
}

void StackyUtil::msg(const String& title, const Char* format, ...) {
    Char msgBuf[4096] = { 0 };
    va_list arglist;
    va_start(arglist, format);
    vswprintf(msgBuf, 4096, format, arglist);
    va_end(arglist);
    MessageBox(nullptr, msgBuf, title.c_str(), MB_OK | MB_ICONINFORMATION);
}
