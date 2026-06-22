#include "app.h"
#include "util.h"

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR cmd_line, _In_ int) {
    HANDLE hMutex = StackyUtil::create_mutex();
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) CloseHandle(hMutex);
        StackyUtil::signal_other_stackies();
        return 0;
    }

    ComInit com;
    String stack_path = StackyUtil::trim(cmd_line, L" \"");

    String title = String(L"StackyNG v") + STACKY_VERSION;

    if (stack_path.empty()) {
        StackyUtil::msg(title, L"Usage: stacky.exe \"C:\\Path\\To\\Your\\Folder\"");
        return ERR_PATH_MISSING;
    }

    if (!fs::is_directory(stack_path)) {
        StackyUtil::msg(title, L"Error: '%s' is not a valid directory.", stack_path.c_str());
        return ERR_PATH_INVALID;
    }

    CacheManager cache(stack_path);
    App app(cache);
    if (app.init()) {
        app.run();
    }

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return 0;
}
