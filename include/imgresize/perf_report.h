#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace imgresize {

/** 单次 resize 的性能打点结果 */
struct ResizePerfPoint {
    int src_w = 0, src_h = 0;
    int dst_w = 0, dst_h = 0;
    int channels = 0;
    double time_ms = 0.0;
    double megapixels_per_sec = 0.0;  // 输出百万像素/秒
};

/** 生成性能分析报告（多组打点汇总） */
std::string generate_perf_report(const std::vector<ResizePerfPoint>& points,
                                 const std::string& title = "Resize Performance Report");

/** 将报告写入文件；目录不存在会尝试创建 */
bool write_perf_report_to_file(const std::string& path,
                                const std::vector<ResizePerfPoint>& points,
                                const std::string& title = "Resize Performance Report");

} // namespace imgresize
