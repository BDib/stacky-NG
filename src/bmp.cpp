#include "bmp.h"
#include "logger.h"

void Buffer::append(const void* src, size_t size) {
    if (size == 0) return;
    const Byte* p = static_cast<const Byte*>(src);
    data.insert(data.end(), p, p + size);
}

void Buffer::append_string(const String& s) {
    append(s.c_str(), (s.size() + 1) * sizeof(Char));
}

bool Buffer::load_from_file(const fs::path& path) {
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"rb") != 0) return false;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data.resize(size);
    if (size > 0) fread(data.data(), 1, size, f);
    fclose(f);
    return true;
}

bool Buffer::save_to_file(const fs::path& path) {
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"wb") != 0) return false;
    if (!data.empty()) fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return true;
}

Bmp::Bmp() {
    memset(&file_header, 0, sizeof(file_header));
    memset(&info_header, 0, sizeof(info_header));
}

Bmp::~Bmp() { if (hBmp) DeleteObject(hBmp); }

Bmp::Bmp(Bmp&& other) noexcept : bits(std::move(other.bits)), hBmp(other.hBmp) {
    file_header = other.file_header;
    info_header = other.info_header;
    other.hBmp = nullptr;
}

Bmp& Bmp::operator=(Bmp&& other) noexcept {
    if (this != &other) {
        if (hBmp) DeleteObject(hBmp);
        file_header = other.file_header;
        info_header = other.info_header;
        bits = std::move(other.bits);
        hBmp = other.hBmp;
        other.hBmp = nullptr;
    }
    return *this;
}

bool Bmp::serialize(Buffer& buf) const {
    buf.append(&file_header, sizeof(file_header));
    buf.append(&info_header, sizeof(info_header));
    buf.append(bits.data(), bits.size());
    return true;
}

bool Bmp::unserialize(const Byte* data, size_t& pos, size_t total_size) {
    if (pos + sizeof(file_header) + sizeof(info_header) > total_size) return false;
    memcpy(&file_header, data + pos, sizeof(file_header));
    pos += sizeof(file_header);
    memcpy(&info_header, data + pos, sizeof(info_header));
    pos += sizeof(info_header);
    size_t bits_size = file_header.bfSize - sizeof(file_header) - sizeof(info_header);
    if (pos + bits_size > total_size) return false;
    bits.assign(data + pos, data + pos + bits_size);
    pos += bits_size;
    return create_hbmp();
}

bool Bmp::create_hbmp() {
    if (hBmp) DeleteObject(hBmp);
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader = info_header;
    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    if (hBmp && pBits) {
        memcpy(pBits, bits.data(), bits.size());
        return true;
    }
    return false;
}

Bmp Bmp::from_file(const fs::path& path) {
    Bmp bmp;
    static IWICImagingFactory* img_factory = nullptr;
    if (!img_factory) {
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&img_factory));
    }
    if (!img_factory) return bmp;

    IWICBitmapDecoder* pDecoder = nullptr;
    if (SUCCEEDED(img_factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder))) {
        IWICBitmapFrameDecode* pFrame = nullptr;
        if (SUCCEEDED(pDecoder->GetFrame(0, &pFrame))) {
            bmp = from_wic_source(pFrame, img_factory);
            pFrame->Release();
        }
        pDecoder->Release();
    }
    return bmp;
}

Bmp Bmp::from_icon(HICON hIcon) {
    Bmp bmp;
    static IWICImagingFactory* img_factory = nullptr;
    if (!img_factory) {
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&img_factory));
    }
    if (!img_factory) return bmp;

    IWICBitmap* pBitmap = nullptr;
    if (SUCCEEDED(img_factory->CreateBitmapFromHICON(hIcon, &pBitmap))) {
        bmp = from_wic_source(pBitmap, img_factory);
        pBitmap->Release();
    }
    return bmp;
}

Bmp Bmp::from_wic_source(IWICBitmapSource* pSource, IWICImagingFactory* img_factory) {
    Bmp bmp;
    UINT cx, cy;
    pSource->GetSize(&cx, &cy);

    const UINT target_size = 32;
    IWICBitmapSource* pFinalSource = nullptr;

    if (cx != target_size || cy != target_size) {
        IWICBitmapScaler* pScaler = nullptr;
        if (SUCCEEDED(img_factory->CreateBitmapScaler(&pScaler))) {
            if (SUCCEEDED(pScaler->Initialize(pSource, target_size, target_size, WICBitmapInterpolationModeLinear))) {
                pScaler->QueryInterface(IID_PPV_ARGS(&pFinalSource));
            }
            pScaler->Release();
        }
    }

    if (!pFinalSource) {
        pSource->QueryInterface(IID_PPV_ARGS(&pFinalSource));
    }

    IWICFormatConverter* pConverter = nullptr;
    if (SUCCEEDED(img_factory->CreateFormatConverter(&pConverter))) {
        if (SUCCEEDED(pConverter->Initialize(pFinalSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom))) {
            if (SUCCEEDED(pConverter->GetSize(&cx, &cy))) {
                UINT stride = cx * 4;
                UINT buf_size = cy * stride;
                bmp.bits.resize(buf_size);
                pConverter->CopyPixels(nullptr, stride, buf_size, bmp.bits.data());

                bmp.info_header.biSize = sizeof(BITMAPINFOHEADER);
                bmp.info_header.biWidth = cx;
                bmp.info_header.biHeight = -(int)cy;
                bmp.info_header.biPlanes = 1;
                bmp.info_header.biBitCount = 32;
                bmp.info_header.biCompression = BI_RGB;

                bmp.file_header.bfType = 0x4d42;
                bmp.file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                bmp.file_header.bfSize = bmp.file_header.bfOffBits + buf_size;

                bmp.create_hbmp();
            }
        }
        pConverter->Release();
    }
    if (pFinalSource) pFinalSource->Release();
    return bmp;
}

HICON Bmp::extract_icon(const fs::path& path) {
    SHFILEINFOW sfi = { 0 };
    HIMAGELIST hIl = (HIMAGELIST)SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
    if (hIl) {
        IImageList* pImageList = nullptr;
        if (SUCCEEDED(SHGetImageList(SHIL_LARGE, IID_PPV_ARGS(&pImageList)))) {
            HICON hIcon = nullptr;
            if (SUCCEEDED(pImageList->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon))) {
                pImageList->Release();
                return hIcon;
            }
            pImageList->Release();
        }
        return ImageList_GetIcon(hIl, sfi.iIcon, ILD_NORMAL);
    }
    return nullptr;
}
