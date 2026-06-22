#include "config.h"

Config Config::load(const fs::path& path) {
    Config config;
    if (!fs::exists(path)) {
        // Generate default config
        std::wofstream ofs(path);
        if (ofs.is_open()) {
            ofs << L"{\n";
            ofs << L"  \"//\": \"StackyNG Configuration\",\n";
            ofs << L"  \"theme\": \"auto\",\n";
            ofs << L"  \"ExampleItem\": {\n";
            ofs << L"    \"name\": \"Example Shortcut\",\n";
            ofs << L"    \"icon\": \"C:/Windows/System32/shell32.dll\",\n";
            ofs << L"    \"args\": \"\",\n";
            ofs << L"    \"admin\": false\n";
            ofs << L"  }\n";
            ofs << L"}\n";
            ofs.close();
        }
    }

    std::wifstream ifs(path);
    if (!ifs.is_open()) return config;

    String content((std::istreambuf_iterator<Char>(ifs)), std::istreambuf_iterator<Char>());

    size_t pos = 0;
    size_t start_brace;
    if (consume(content, pos, L"{", start_brace)) pos = start_brace;

    while (pos < content.size()) {
        size_t next_pos;
        String key = parse_token(content, pos, next_pos);
        if (key.empty()) break;
        pos = next_pos;

        if (key == L"theme") {
            consume(content, pos, L":", next_pos);
            config.theme = parse_token(content, next_pos, pos);
        } else {
            consume(content, pos, L":", next_pos);
            pos = next_pos;
            consume(content, pos, L"{", next_pos);
            pos = next_pos;

            ItemConfig item;
            while (pos < content.size()) {
                String sub_key = parse_token(content, pos, next_pos);
                if (sub_key.empty()) break;
                pos = next_pos;

                consume(content, pos, L":", next_pos);
                pos = next_pos;

                if (sub_key == L"name") item.name = parse_token(content, pos, next_pos);
                else if (sub_key == L"icon") item.icon = parse_token(content, pos, next_pos);
                else if (sub_key == L"args") item.args = parse_token(content, pos, next_pos);
                else if (sub_key == L"admin") {
                    String val = parse_token(content, pos, next_pos);
                    item.admin = (val == L"true");
                }
                pos = next_pos;

                size_t comma_pos;
                if (consume(content, pos, L",", comma_pos)) pos = comma_pos;
                size_t close_pos;
                if (consume(content, pos, L"}", close_pos)) {
                    pos = close_pos;
                    break;
                }
            }
            config.items[key] = item;
        }
    }

    return config;
}

const ItemConfig* Config::get_item(const String& name) const {
    auto it = items.find(name);
    return it != items.end() ? &it->second : nullptr;
}

String Config::parse_token(const String& s, size_t pos, size_t& next_pos) {
    while (pos < s.size() && iswspace(s[pos])) pos++;
    if (pos >= s.size()) return L"";

    if (s[pos] == L'\"') {
        size_t end = s.find(L'\"', pos + 1);
        if (end == String::npos) return L"";
        next_pos = end + 1;
        return s.substr(pos + 1, end - pos - 1);
    } else {
        size_t end = pos;
        while (end < s.size() && !iswspace(s[end]) && s[end] != L':' && s[end] != L',' && s[end] != L'}' && s[end] != L'{') end++;
        next_pos = end;
        return s.substr(pos, end - pos);
    }
}

bool Config::consume(const String& s, size_t pos, const Char* token, size_t& next_pos) {
    while (pos < s.size() && iswspace(s[pos])) pos++;
    if (pos < s.size() && s[pos] == token[0]) {
        next_pos = pos + 1;
        return true;
    }
    return false;
}
