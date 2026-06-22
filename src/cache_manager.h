#pragma once
#include "bmp.h"
#include "config.h"

struct CacheItem {
    String name;
    bool is_dir;
    Bmp bmp;
    String custom_name;
    String args;
    String description;
    bool admin = false;
    std::vector<CacheItem> children;

    void serialize(Buffer& buf) const;
    bool unserialize(const Byte* data, size_t& pos, size_t total_size);

private:
    bool unserialize_string(const Byte* data, size_t& pos, size_t total_size, String& out);
};

class CacheManager {
public:
    CacheItem root;
    bool was_rebuilt = false;
    std::mutex mtx;
    bool is_loading = false;
    Config root_config;

    CacheManager(const String& stack_path);
    void load_async(HWND notify_window);
    bool load_internal();
    fs::path get_full_path(const String& relative) const;
    const fs::path& get_base_dir() const { return base_dir; }

private:
    bool is_same_structure(const CacheItem& a, const CacheItem& b);
    bool scan_recursive(const fs::path& dir, std::vector<CacheItem>& items, Time& max_modified, std::set<fs::path>& visited);
    void rebuild_icons(CacheItem& item, const fs::path& current_path);
    void save();

    fs::path base_dir;
    fs::path cache_path;
};
