#include "cache_manager.h"
#include "util.h"
#include "logger.h"

void CacheItem::serialize(Buffer& buf) const {
    buf.append_string(name);
    Byte bDir = is_dir ? 1 : 0;
    buf.append(&bDir, 1);
    bmp.serialize(buf);
    buf.append_string(custom_name);
    buf.append_string(args);
    buf.append_string(description);
    Byte bAdmin = admin ? 1 : 0;
    buf.append(&bAdmin, 1);

    uint32_t child_count = static_cast<uint32_t>(children.size());
    buf.append(&child_count, sizeof(child_count));
    for (const auto& child : children) {
        child.serialize(buf);
    }
}

bool CacheItem::unserialize(const Byte* data, size_t& pos, size_t total_size) {
    if (!unserialize_string(data, pos, total_size, name)) return false;

    if (pos >= total_size) return false;
    is_dir = data[pos++] != 0;
    if (!bmp.unserialize(data, pos, total_size)) return false;

    if (!unserialize_string(data, pos, total_size, custom_name)) return false;
    if (!unserialize_string(data, pos, total_size, args)) return false;
    if (!unserialize_string(data, pos, total_size, description)) return false;

    if (pos >= total_size) return false;
    admin = data[pos++] != 0;

    if (pos + sizeof(uint32_t) > total_size) return false;
    uint32_t child_count;
    memcpy(&child_count, data + pos, sizeof(child_count));
    pos += sizeof(child_count);
    children.clear();
    for (uint32_t i = 0; i < child_count; ++i) {
        CacheItem child;
        if (!child.unserialize(data, pos, total_size)) return false;
        children.push_back(std::move(child));
    }
    return true;
}

bool CacheItem::unserialize_string(const Byte* data, size_t& pos, size_t total_size, String& out) {
    if (pos + sizeof(Char) > total_size) return false;
    const Char* pStr = reinterpret_cast<const Char*>(data + pos);
    size_t max_chars = (total_size - pos) / sizeof(Char);
    size_t len = 0;
    while (len < max_chars && pStr[len] != 0) len++;
    if (len >= max_chars) return false;
    out.assign(pStr, len);
    pos += (len + 1) * sizeof(Char);
    return true;
}

CacheManager::CacheManager(const String& stack_path) : base_dir(StackyUtil::trim(stack_path, L" \"")) {
    cache_path = base_dir / CACHE_FILE_NAME;
    Logger::init(base_dir / L"!stacky.log");
    LOG_INFO("CacheManager initialized for path: %ls", base_dir.c_str());
    root_config = Config::load(base_dir / L"stacky.json");
}

void CacheManager::load_async(HWND notify_window) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (is_loading) return;
        is_loading = true;
    }
    std::thread([this, notify_window]() {
        ComInit com;
        bool success = load_internal();
        {
            std::lock_guard<std::mutex> lock(mtx);
            is_loading = false;
        }
        PostMessage(notify_window, WM_CACHE_READY, success ? 1 : 0, 0);
    }).detach();
}

bool CacheManager::load_internal() {
    Time last_modified = StackyUtil::get_modified(cache_path);
    Time current_max_modified = 0;

    std::vector<CacheItem> temp_children;
    std::set<fs::path> visited;
    bool has_links = scan_recursive(base_dir, temp_children, current_max_modified, visited);
    if (!has_links) {
        LOG_WARN("No .lnk files found in %ls", base_dir.c_str());
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        root.children = std::move(temp_children);
    }

    MappedFile mf;
    bool loaded = false;
    if (last_modified > 0 && last_modified >= current_max_modified && mf.open(cache_path)) {
        size_t pos = 0;
        CacheItem cached_root;
        if (cached_root.unserialize(static_cast<const Byte*>(mf.pData), pos, mf.size)) {
            std::lock_guard<std::mutex> lock(mtx);
            if (is_same_structure(root, cached_root)) {
                root = std::move(cached_root);
                loaded = true;
            }
        }
        mf.close();
    }

    if (!loaded) {
        LOG_INFO("Cache outdated or missing, rebuilding...");
        {
            std::lock_guard<std::mutex> lock(mtx);
            rebuild_icons(root, base_dir);
            HICON hRootIcon = Bmp::extract_icon(base_dir);
            if (hRootIcon) {
                root.bmp = Bmp::from_icon(hRootIcon);
                DestroyIcon(hRootIcon);
            }
            save();
            was_rebuilt = true;
        }
    } else {
        std::lock_guard<std::mutex> lock(mtx);
        if (!root.bmp.hBmp) {
            HICON hRootIcon = Bmp::extract_icon(base_dir);
            if (hRootIcon) {
                root.bmp = Bmp::from_icon(hRootIcon);
                DestroyIcon(hRootIcon);
            }
        }
    }

    return true;
}

fs::path CacheManager::get_full_path(const String& relative) const {
    return base_dir / relative;
}

bool CacheManager::is_same_structure(const CacheItem& a, const CacheItem& b) {
    if (a.children.size() != b.children.size()) return false;
    for (size_t i = 0; i < a.children.size(); ++i) {
        if (a.children[i].name != b.children[i].name) return false;
        if (a.children[i].is_dir != b.children[i].is_dir) return false;
        if (!is_same_structure(a.children[i], b.children[i])) return false;
    }
    return true;
}

bool CacheManager::scan_recursive(const fs::path& dir, std::vector<CacheItem>& items, Time& max_modified, std::set<fs::path>& visited) {
    bool found_link = false;
    Config config = Config::load(dir / L"stacky.json");
    max_modified = (std::max)(max_modified, StackyUtil::get_modified(dir / L"stacky.json"));

    try {
        fs::path canonical_dir = fs::canonical(dir);
        if (visited.count(canonical_dir)) {
            LOG_WARN("Circular junction or symlink detected at %ls", dir.c_str());
            return false;
        }
        visited.insert(canonical_dir);

        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_directory()) {
                DWORD attrs = GetFileAttributesW(entry.path().c_str());
                if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_HIDDEN)) continue;

                std::vector<CacheItem> sub_items;
                if (scan_recursive(entry.path(), sub_items, max_modified, visited)) {
                    CacheItem item;
                    item.name = entry.path().filename().wstring();
                    item.is_dir = true;
                    item.children = std::move(sub_items);
                    items.push_back(std::move(item));
                    found_link = true;
                    max_modified = (std::max)(max_modified, StackyUtil::get_modified(entry.path()));
                }
            } else if (entry.is_regular_file()) {
                if (_wcsicmp(entry.path().extension().c_str(), L".lnk") == 0) {
                    String file_name = entry.path().filename().wstring();
                    String display_name = file_name;
                    if (display_name.size() > 4 && _wcsicmp(display_name.substr(display_name.size() - 4).c_str(), L".lnk") == 0) {
                        display_name = display_name.substr(0, display_name.size() - 4);
                    }

                    const ItemConfig* item_cfg = config.get_item(display_name);

                    CacheItem item;
                    item.name = file_name;
                    item.is_dir = false;
                    item.description = StackyUtil::get_description(entry.path());
                    if (item_cfg) {
                        item.custom_name = item_cfg->name;
                        item.args = item_cfg->args;
                        item.admin = item_cfg->admin;
                    }

                    items.push_back(std::move(item));
                    found_link = true;
                    max_modified = (std::max)(max_modified, StackyUtil::get_modified(entry.path()));
                }
            }
        }
    } catch (...) {
    }

    std::sort(items.begin(), items.end(), [](const CacheItem& a, const CacheItem& b) {
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
    });

    return found_link;
}

void CacheManager::rebuild_icons(CacheItem& item, const fs::path& current_path) {
    Config config = Config::load(current_path / L"stacky.json");

    for (auto& child : item.children) {
        fs::path child_path = current_path / child.name;

        String display_name = child.name;
        if (display_name.size() > 4 && _wcsicmp(display_name.substr(display_name.size() - 4).c_str(), L".lnk") == 0) {
            display_name = display_name.substr(0, display_name.size() - 4);
        }
        const ItemConfig* item_cfg = config.get_item(display_name);

        bool loaded = false;
        if (item_cfg && !item_cfg->icon.empty()) {
            fs::path icon_path = item_cfg->icon;
            if (icon_path.is_relative()) icon_path = current_path / icon_path;
            child.bmp = Bmp::from_file(icon_path);
            if (child.bmp.hBmp) loaded = true;
        }

        if (!loaded) {
            HICON hIcon = Bmp::extract_icon(child_path);
            if (hIcon) {
                child.bmp = Bmp::from_icon(hIcon);
                DestroyIcon(hIcon);
            } else {
                LOG_ERR("Failed to extract icon for %ls", child_path.c_str());
                HICON hDefault = LoadIcon(nullptr, IDI_APPLICATION);
                if (hDefault) {
                    child.bmp = Bmp::from_icon(hDefault);
                }
            }
        }
        if (child.is_dir) {
            rebuild_icons(child, child_path);
        }
    }
}

void CacheManager::save() {
    Buffer buf;
    root.serialize(buf);
    buf.save_to_file(cache_path);
    SetFileAttributesW(cache_path.c_str(), FILE_ATTRIBUTE_HIDDEN);
}
