/**
 * @file perf_report.cpp
 * @brief 性能分析报告生成与写入
 */

#include "imgresize/perf_report.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
inline bool make_dir(const char* path) { return _mkdir(path) == 0 || (_access(path, 0) == 0); }
#else
#include <sys/stat.h>
#include <unistd.h>
inline bool make_dir(const char* path) { return mkdir(path, 0755) == 0 || (access(path, F_OK) == 0); }
#endif

namespace imgresize {

std::string generate_perf_report(const std::vector<ResizePerfPoint>& points,
                                  const std::string& title) {
    std::ostringstream os;
    os << "========================================\n";
    os << title << "\n";
    os << "========================================\n\n";

    os << std::fixed << std::setprecision(2);
    os << std::setw(12) << "src_size"
       << std::setw(14) << "dst_size"
       << std::setw(10) << "ch"
       << std::setw(12) << "time_ms"
       << std::setw(16) << "Mpix/s"
       << "\n";
    os << std::string(64, '-') << "\n";

    for (const auto& p : points) {
        std::string src_sz = std::to_string(p.src_w) + "x" + std::to_string(p.src_h);
        std::string dst_sz = std::to_string(p.dst_w) + "x" + std::to_string(p.dst_h);
        os << std::setw(12) << src_sz
           << std::setw(14) << dst_sz
           << std::setw(10) << p.channels
           << std::setw(12) << p.time_ms
           << std::setw(16) << p.megapixels_per_sec
           << "\n";
    }
    os << "\n";
    return os.str();
}

static bool ensure_parent_dir(const std::string& path) {
    const std::string::size_type i = path.find_last_of("/\\");
    if (i == std::string::npos || i == 0) return true;
    std::string dir = path.substr(0, i);
    return make_dir(dir.c_str());
}

bool write_perf_report_to_file(const std::string& path,
                                const std::vector<ResizePerfPoint>& points,
                                const std::string& title) {
    if (!ensure_parent_dir(path)) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << generate_perf_report(points, title);
    return f.good();
}

} // namespace imgresize
