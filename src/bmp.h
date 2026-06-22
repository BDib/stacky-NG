#pragma once
#include "common.h"

struct Buffer {
    std::vector<Byte> data;

    void append(const void* src, size_t size);
    void append_string(const String& s);
    bool load_from_file(const fs::path& path);
    bool save_to_file(const fs::path& path);
};

struct Bmp {
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    std::vector<Byte> bits;
    HBITMAP hBmp = nullptr;

    Bmp();
    ~Bmp();
    Bmp(const Bmp&) = delete;
    Bmp& operator=(const Bmp&) = delete;
    Bmp(Bmp&& other) noexcept;
    Bmp& operator=(Bmp&& other) noexcept;

    bool serialize(Buffer& buf) const;
    bool unserialize(const Byte* data, size_t& pos, size_t total_size);
    bool create_hbmp();

    static Bmp from_file(const fs::path& path);
    static Bmp from_icon(HICON hIcon);
    static Bmp from_wic_source(IWICBitmapSource* pSource, IWICImagingFactory* img_factory);
    static HICON extract_icon(const fs::path& path);
};
