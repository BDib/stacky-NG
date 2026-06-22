#pragma once
#include "cache_manager.h"

struct App {
    HWND window = nullptr;
    HWND tooltip = nullptr;
    CacheManager& cache;
    int next_cmd_id = WM_MENU_ITEM;
    struct TargetInfo {
        String path;
        String args;
        String description;
        bool admin;
    };
    std::vector<TargetInfo> id_to_target;
    bool switching_menus = false;
    HANDLE hDirChange = INVALID_HANDLE_VALUE;
    std::thread watch_thread;
    std::atomic<bool> stop_watching = false;
    std::chrono::steady_clock::time_point last_reload;

    HMENU pending_menu = nullptr;
    bool is_menu_active = false;
    POINT menu_pos = { 0, 0 };

    App(CacheManager& c);
    ~App();
    bool init();
    void on_cache_ready(bool success);
    void build_menu(HMENU hMenu, const CacheItem& parent, const fs::path& current_path);
    void show(HMENU hMenu, bool is_refresh);
    void start_watching();
    void run();

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
