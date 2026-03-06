/**
 * @file resize_png.cpp
 * @brief 命令行工具：resize_png input.png output.png w h
 *
 * 使用 Windows GDI+ 读写 PNG，将像素数据转换为 imgresize::ImageBuf，
 * 调用 resize_bilinear 完成缩放，再写回 PNG。
 */

#include "imgresize/resize.h"
#include "imgresize/types.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#  include <windows.h>
#  include <gdiplus.h>
   #pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;
#endif

using namespace imgresize;

#ifdef _WIN32

// 简单的窄字符到宽字符转换（仅适用于 ASCII/UTF-8 基本字符）
static std::wstring to_wstring(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

// 获取 PNG 编码器 CLSID
static bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;    // image encoders 数量
    UINT size = 0;   // image encoder 数组大小（字节）

    GetImageEncodersSize(&num, &size);
    if (size == 0) return false;

    std::vector<BYTE> buffer(size);
    ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(buffer.data());
    if (GetImageEncoders(num, size, pImageCodecInfo) != Ok) return false;

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            return true;
        }
    }
    return false;
}

// 使用 GDI+ 从 PNG 加载到 ImageBuf（float RGBA 0..1）
static bool load_png_gdiplus(const std::string& filename, ImageBuf& out) {
    std::wstring wpath = to_wstring(filename);
    std::unique_ptr<Bitmap> bmp(Bitmap::FromFile(wpath.c_str(), FALSE));
    if (!bmp || bmp->GetLastStatus() != Ok) {
        return false;
    }

    const int w = static_cast<int>(bmp->GetWidth());
    const int h = static_cast<int>(bmp->GetHeight());

    Rect rect(0, 0, w, h);
    BitmapData data;
    if (bmp->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data) != Ok) {
        return false;
    }

    out.width = w;
    out.height = h;
    out.channels = 4; // RGBA
    out.data.resize(static_cast<std::size_t>(w) * h * out.channels);

    const int stride = data.Stride;
    auto* scan0 = static_cast<BYTE*>(data.Scan0);
    for (int y = 0; y < h; ++y) {
        BYTE* row = scan0 + y * stride;
        for (int x = 0; x < w; ++x) {
            BYTE B = row[x * 4 + 0];
            BYTE G = row[x * 4 + 1];
            BYTE R = row[x * 4 + 2];
            BYTE A = row[x * 4 + 3];
            std::size_t idx = (static_cast<std::size_t>(y) * w + x) * 4;
            out.data[idx + 0] = R / 255.0f;
            out.data[idx + 1] = G / 255.0f;
            out.data[idx + 2] = B / 255.0f;
            out.data[idx + 3] = A / 255.0f;
        }
    }

    bmp->UnlockBits(&data);
    return true;
}

// 使用 GDI+ 将 ImageBuf 写为 PNG
static bool save_png_gdiplus(const std::string& filename, const ImageBuf& img) {
    if (img.empty() || img.channels != 4) return false;

    const int w = img.width;
    const int h = img.height;
    Bitmap bmp(w, h, PixelFormat32bppARGB);

    Rect rect(0, 0, w, h);
    BitmapData data;
    if (bmp.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &data) != Ok) {
        return false;
    }

    const int stride = data.Stride;
    auto* scan0 = static_cast<BYTE*>(data.Scan0);
    for (int y = 0; y < h; ++y) {
        BYTE* row = scan0 + y * stride;
        for (int x = 0; x < w; ++x) {
            std::size_t idx = (static_cast<std::size_t>(y) * w + x) * 4;
            auto clamp = [](float v) -> BYTE {
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;
                return static_cast<BYTE>(v * 255.0f + 0.5f);
            };
            BYTE R = clamp(img.data[idx + 0]);
            BYTE G = clamp(img.data[idx + 1]);
            BYTE B = clamp(img.data[idx + 2]);
            BYTE A = clamp(img.data[idx + 3]);
            row[x * 4 + 0] = B;
            row[x * 4 + 1] = G;
            row[x * 4 + 2] = R;
            row[x * 4 + 3] = A;
        }
    }

    bmp.UnlockBits(&data);

    CLSID clsid;
    if (!GetEncoderClsid(L"image/png", &clsid)) {
        return false;
    }

    std::wstring wpath = to_wstring(filename);
    return bmp.Save(wpath.c_str(), &clsid, nullptr) == Ok;
}

#endif // _WIN32

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    if (argc != 5) {
        std::cerr << "用法: resize_png input.png output.png width height\n";
        return 1;
    }

    std::string input  = argv[1];
    std::string output = argv[2];
    int dst_w = std::atoi(argv[3]);
    int dst_h = std::atoi(argv[4]);
    if (dst_w <= 0 || dst_h <= 0) {
        std::cerr << "width/height 必须为正整数\n";
        return 1;
    }

#ifndef _WIN32
    std::cerr << "当前示例使用 Windows GDI+ 读写 PNG，仅在 Windows 上可用。\n";
    return 1;
#else
    // 初始化 GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok) {
        std::cerr << "GDI+ 初始化失败\n";
        return 1;
    }

    ImageBuf src, dst;
    if (!load_png_gdiplus(input, src)) {
        std::cerr << "读取 PNG 失败: " << input << "\n";
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    ResizePerfPoint perf;
    if (!resize_bilinear(src, dst, dst_w, dst_h, 4, &perf)) {
        std::cerr << "resize_bilinear 失败\n";
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    if (!save_png_gdiplus(output, dst)) {
        std::cerr << "写入 PNG 失败: " << output << "\n";
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    std::cout << "OK: " << input << " -> " << output
              << " (" << src.width << "x" << src.height
              << " -> " << dst.width << "x" << dst.height << ")\n";
    std::cout << "time = " << perf.time_ms
              << " ms, Mpix/s = " << perf.megapixels_per_sec << "\n";

    GdiplusShutdown(gdiplusToken);
    return 0;
#endif
}

