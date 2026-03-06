/**
 * @file bench_resize.cpp
 * @brief 对 resize_bilinear 做多组尺寸的基准测试，并生成性能分析报告
 */

#include "imgresize/resize.h"
#include "imgresize/perf_report.h"
#include "imgresize/types.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace imgresize;

static ImageBuf make_random_image(int w, int h, int c) {
    ImageBuf img;
    img.width = w;
    img.height = h;
    img.channels = c;
    img.data.resize(static_cast<std::size_t>(w) * h * c);
    for (std::size_t i = 0; i < img.data.size(); ++i)
        img.data[i] = static_cast<float>(rand() % 256) / 255.f;
    return img;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "========== imgresize 缩放性能基准 ==========\n\n";

    struct SizePair { int src_w, src_h, dst_w, dst_h; };
    std::vector<SizePair> cases = {
        { 640,  480,  320,  240  },
        { 1920, 1080, 960,  540  },
        { 1920, 1080, 1920, 1080 },
        { 3840, 2160, 1920, 1080 },
        { 800,  600,  400,  300  },
    };

    std::vector<ResizePerfPoint> points;
    points.reserve(cases.size());

    for (const auto& s : cases) {
        ImageBuf src = make_random_image(s.src_w, s.src_h, 3);
        ImageBuf dst;
        ResizePerfPoint perf;
        if (!resize_bilinear(src, dst, s.dst_w, s.dst_h, 3, &perf)) {
            std::cerr << "resize failed: " << s.src_w << "x" << s.src_h << " -> " << s.dst_w << "x" << s.dst_h << "\n";
            continue;
        }
        points.push_back(perf);
    }

    std::string report = generate_perf_report(points, "imgresize resize_bilinear 性能报告");
    std::cout << report;

    /* 写入报告文件 */
    const std::string report_path = "reports/resize_perf_report.txt";
    if (write_perf_report_to_file(report_path, points, "imgresize resize_bilinear 性能报告")) {
        std::cout << "报告已写入: " << report_path << "\n";
    } else {
        std::cout << "报告未写入文件（可手动创建 reports 目录后重试）\n";
    }

    return 0;
}
