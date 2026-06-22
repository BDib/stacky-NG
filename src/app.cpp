#include "app.h"
#include "util.h"
#include "logger.h"

App::App(CacheManager& c) : cache(c) {
    last_reload = std::chrono::steady_clock::now();
}

App::~App() {
    if (tooltip) DestroyWindow(tooltip);
    stop_watching = true;
    if (hDirChange != INVALID_HANDLE_VALUE) {
        CancelIoEx(hDirChange, nullptr);
        CloseHandle(hDirChange);
    }
    if (watch_thread.joinable()) watch_thread.join();
}

bool App::init() {
    bool dark = false;
    if (cache.root_config.theme == L"dark") dark = true;
    else if (cache.root_config.theme == L"light") dark = false;
    else dark = StackyUtil::is_dark_mode();

    StackyUtil::signal_other_stackies();
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = STACKY_WINDOW_NAME;
    RegisterClass(&wc);

    window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, STACKY_WINDOW_NAME, STACKY_WINDOW_NAME, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!window) return false;
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(window, SW_HIDE);

    StackyUtil::set_dark_mode(window, dark);

    tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, window, nullptr, GetModuleHandle(nullptr), nullptr);
    if (tooltip) {
        SendMessage(tooltip, TTM_SETMAXTIPWIDTH, 0, 400);
        StackyUtil::set_dark_mode(tooltip, dark);
    }

    cache.load_async(window);

    HMENU hLoadingMenu = CreatePopupMenu();
    AppendMenu(hLoadingMenu, MF_GRAYED, 0, L"Loading...");
    show(hLoadingMenu, false);
    DestroyMenu(hLoadingMenu);

    return true;
}

void App::on_cache_ready(bool success) {
    if (!success) {
        LOG_ERR("Failed to load cache.");
        PostQuitMessage(0);
        return;
    }

    HMENU hMenu = CreatePopupMenu();

    {
        std::lock_guard<std::mutex> lock(cache.mtx);
        String openLabel = L"Open:  " + cache.get_base_dir().filename().wstring();
        AppendMenu(hMenu, MF_STRING, WM_OPEN_TARGET_FOLDER, openLabel.c_str());

        MENUITEMINFO mii = { sizeof(mii) };
        mii.fMask = MIIM_BITMAP;
        mii.hbmpItem = cache.root.bmp.hBmp;
        SetMenuItemInfo(hMenu, 0, TRUE, &mii);

        if (cache.was_rebuilt) {
            AppendMenu(hMenu, MF_GRAYED, 0, L"Stack cache rebuilt!");
        }
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

        build_menu(hMenu, cache.root, cache.get_base_dir());
    }

    // Bug Fix: Trigger menu switch via message to avoid recursion and ensure clean state
    PostMessage(window, WM_SHOW_MENU, (WPARAM)hMenu, 0);
}

void App::build_menu(HMENU hMenu, const CacheItem& parent, const fs::path& current_path) {
    for (const auto& item : parent.children) {
        fs::path item_path = current_path / item.name;
        String display_name = item.name;

        if (display_name.size() > 4 && _wcsicmp(display_name.substr(display_name.size() - 4).c_str(), L".lnk") == 0) {
            display_name = display_name.substr(0, display_name.size() - 4);
        }

        String display_label = item.custom_name.empty() ? display_name : item.custom_name;

        if (item.is_dir) {
            HMENU hSubMenu = CreatePopupMenu();
            build_menu(hSubMenu, item, item_path);

            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, (display_label + L"  >").c_str());

            MENUITEMINFO mii = { sizeof(mii) };
            mii.fMask = MIIM_BITMAP;
            mii.hbmpItem = item.bmp.hBmp;
            SetMenuItemInfo(hMenu, GetMenuItemCount(hMenu) - 1, TRUE, &mii);
        } else {
            int id = next_cmd_id++;
            TargetInfo target;
            target.path = item_path.wstring();
            target.args = item.args;
            target.admin = item.admin;
            target.description = item.description;

            id_to_target.push_back(target);
            AppendMenu(hMenu, MF_STRING, id, display_label.c_str());

            MENUITEMINFO mii = { sizeof(mii) };
            mii.fMask = MIIM_BITMAP;
            mii.hbmpItem = item.bmp.hBmp;
            SetMenuItemInfo(hMenu, GetMenuItemCount(hMenu) - 1, TRUE, &mii);
        }
    }
}

void App::show(HMENU hMenu, bool is_refresh) {
    if (stop_watching) return;

    if (!is_refresh) {
        GetCursorPos(&menu_pos);
    }

    start_watching();
    SetForegroundWindow(window);

    is_menu_active = true;
    TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_VERPOSANIMATION, menu_pos.x, menu_pos.y, window, nullptr);
    is_menu_active = false;
}

void App::start_watching() {
    if (hDirChange != INVALID_HANDLE_VALUE) return;

    hDirChange = CreateFileW(cache.get_base_dir().c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (hDirChange == INVALID_HANDLE_VALUE) return;

    stop_watching = false;
    watch_thread = std::thread([this]() {
        alignas(DWORD) Byte buffer[4096];
        DWORD bytesReturned;
        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        while (!stop_watching) {
            if (ReadDirectoryChangesW(hDirChange, buffer, sizeof(buffer), TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                nullptr, &overlapped, nullptr)) {

                WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (stop_watching) break;

                if (GetOverlappedResult(hDirChange, &overlapped, &bytesReturned, FALSE)) {
                    bool relevant_change = false;
                    PFILE_NOTIFY_INFORMATION pNotify = (PFILE_NOTIFY_INFORMATION)buffer;
                    do {
                        String fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(wchar_t));
                        if (_wcsicmp(fileName.c_str(), CACHE_FILE_NAME.c_str()) != 0 &&
                            _wcsicmp(fileName.c_str(), L"!stacky.log") != 0) {
                            relevant_change = true;
                            break;
                        }
                        pNotify = pNotify->NextEntryOffset ? (PFILE_NOTIFY_INFORMATION)((Byte*)pNotify + pNotify->NextEntryOffset) : nullptr;
                    } while (pNotify);

                    if (relevant_change) {
                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_reload).count() > 500) {
                            LOG_INFO("Relevant change detected in directory, triggering async reload.");
                            cache.load_async(window);
                            last_reload = now;
                        }
                    }
                }
                ResetEvent(overlapped.hEvent);
            } else {
                break;
            }
        }
        CloseHandle(overlapped.hEvent);
    });
}

LRESULT CALLBACK App::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    App* app = (App*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
    case WM_SHOW_MENU:
        if (app) {
            app->switching_menus = true;
            if (app->is_menu_active) {
                app->pending_menu = (HMENU)wparam;
                EndMenu();
            } else {
                app->show((HMENU)wparam, true);
                DestroyMenu((HMENU)wparam);
            }
            app->switching_menus = false;
        }
        break;
    case WM_MENUSELECT: {
        if (app && app->tooltip) {
            UINT item_id = (UINT)LOWORD(wparam);
            UINT flags = (UINT)HIWORD(wparam);
            HMENU hMenu = (HMENU)lparam;

            if (!(flags & MF_POPUP) && item_id >= WM_MENU_ITEM && hMenu) {
                size_t idx = (size_t)item_id - WM_MENU_ITEM;
                if (idx < app->id_to_target.size()) {
                    const TargetInfo& target = app->id_to_target[idx];

                    TOOLINFOW ti = { sizeof(ti) };
                    ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
                    ti.hwnd = hwnd;
                    String tip_text = target.description.empty() ? target.path : target.description;
                    ti.lpszText = (LPWSTR)tip_text.c_str();
                    SendMessage(app->tooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
                    SendMessage(app->tooltip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);

                    POINT pt;
                    GetCursorPos(&pt);
                    SendMessage(app->tooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 20, pt.y + 10));
                    SendMessage(app->tooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
                }
            } else {
                TOOLINFOW ti = { sizeof(ti) };
                ti.hwnd = hwnd;
                SendMessage(app->tooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
            }
        }
        break;
    }
    case WM_CACHE_READY:
        if (app) {
            app->next_cmd_id = WM_MENU_ITEM;
            app->id_to_target.clear();
            app->on_cache_ready(wparam != 0);
        }
        break;
    case WM_COMMAND: {
        if (!app) break;
        if (wparam == WM_OPEN_TARGET_FOLDER) {
            if (lparam == 1) {
                app->cache.load_async(hwnd);
                return 0;
            }
            ShellExecuteW(nullptr, nullptr, app->cache.get_base_dir().c_str(), nullptr, nullptr, SW_NORMAL);
            PostQuitMessage(0); // Exit immediately after launching folder
        } else {
            size_t idx = static_cast<size_t>(wparam) - WM_MENU_ITEM;
            if (idx < app->id_to_target.size()) {
                const TargetInfo& target = app->id_to_target[idx];
                SHELLEXECUTEINFOW sei = { sizeof(sei) };
                sei.fMask = SEE_MASK_DEFAULT;
                sei.lpVerb = target.admin ? L"runas" : nullptr;
                sei.lpFile = target.path.c_str();
                sei.lpParameters = target.args.empty() ? nullptr : target.args.c_str();
                sei.nShow = SW_NORMAL;
                if (!ShellExecuteExW(&sei)) {
                    LOG_ERR("Failed to launch %ls: %d", target.path.c_str(), GetLastError());
                }
                    PostQuitMessage(0); // Exit immediately after launching target
            }
        }
        break;
    }
    case WM_EXITMENULOOP:
        if (app && !app->switching_menus) {
            if (app->pending_menu) {
                HMENU hMenu = app->pending_menu;
                app->pending_menu = nullptr;
                app->show(hMenu, true);
                DestroyMenu(hMenu);
            } else {
                // Bug Fix: Exit immediately when menu is closed unless a command is already posted
                PostQuitMessage(0);
            }
        }
        break;
    case WM_ENTERMENULOOP:
        if (app) {
            KillTimer(hwnd, 0);
        }
        break;
    case WM_TIMER:
        if (app && !app->switching_menus) {
            if (app->tooltip) {
                TOOLINFOW ti = { sizeof(ti) };
                ti.hwnd = hwnd;
                SendMessage(app->tooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
            }
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void App::run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
