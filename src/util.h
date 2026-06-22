#pragma once
#include "common.h"

struct MappedFile {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = nullptr;
    void* pData = nullptr;
    size_t size = 0;

    ~MappedFile() { close(); }
    bool open(const fs::path& path);
    void close();
};

struct StackyUtil {
    static String get_description(const fs::path& path);
    static bool is_dark_mode();
    static void set_dark_mode(HWND hwnd, bool dark);
    static String trim(const String& target, const String& trim_chars);
    static void signal_other_stackies();
    static HANDLE create_mutex();
    static Time get_modified(const fs::path& path);
    static void msg(const String& title, const Char* format, ...);
};
