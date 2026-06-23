#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>
#include <wincodec.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shlguid.h>
#include <uxtheme.h>
#include <dwmapi.h>

// Link necessary libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <chrono>
#include <fstream>
#include <ctime>
#include <thread>
#include <iterator>
#include <stdarg.h>
#include <future>
#include <mutex>
#include <set>
#include <map>
#include <atomic>

namespace fs = std::filesystem;

typedef wchar_t                 Char;
typedef unsigned char           Byte;
typedef __time64_t              Time;
typedef std::wstring            String;

inline const String CACHE_FILE_NAME    = L"!stacky.cache";
inline const String STACKY_EXEC_NAME   = L"stacky.exe";
inline const Char*  STACKY_WINDOW_NAME = L"stackyNG";
inline const String STACKY_VERSION     = L"0.7.0.4";

enum {
    WM_BASE                     = WM_USER + 100,
    WM_OPEN_TARGET_FOLDER       = WM_BASE + 1,
    WM_CACHE_READY              = WM_BASE + 2,
    WM_SHOW_MENU                = WM_BASE + 3,
    WM_MENU_ITEM                = WM_BASE + 100,

    APP_EXIT_DELAY              = 3 * 1000,

    ERR_PATH_MISSING            = 401,
    ERR_PATH_INVALID            = 402,
};

struct ComInit {
    ComInit() {
        HRESULT hr = CoInitialize(nullptr);
        (void)hr;
    }
    ~ComInit() { CoUninitialize(); }
};
