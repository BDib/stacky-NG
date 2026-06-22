#pragma once
#include "common.h"

struct ItemConfig {
    String name;
    String icon;
    String args;
    bool admin = false;
};

struct Config {
    std::map<String, ItemConfig> items;
    String theme = L"auto"; // auto, dark, light

    static Config load(const fs::path& path);
    const ItemConfig* get_item(const String& name) const;

private:
    static String parse_token(const String& s, size_t pos, size_t& next_pos);
    static bool consume(const String& s, size_t pos, const Char* token, size_t& next_pos);
};
