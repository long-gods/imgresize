/**
 * @file resize.cpp
 * @brief 双线性插值图像缩放实现，含性能打点
 */

#include "imgresize/resize.h"
#include "imgresize/perf_report.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstring>

#ifdef IMGRESIZE_USE_OPENMP
#include <omp.h>
#endif

namespace imgresize {

namespace {

/**
 * 将源图坐标映射到目标图坐标的缩放比例
 * 使用中心对齐：输出像素 (dx, dy) 中心 对应 源图 (sx, sy) 中心
 * sx = (dx + 0.5) * scale_x - 0.5
 * sy = (dy + 0.5) * scale_y - 0.5
 */
inline void scale_factors(int src_w, int src_h, int dst_w, int dst_h,
                         double& scale_x, double& scale_y) {
    scale_x = (dst_w > 1) ? static_cast<double>(src_w) / dst_w : 0.0;
    scale_y = (dst_h > 1) ? static_cast<double>(src_h) / dst_h : 0.0;
}

/**
 * 双线性插值：根据 (sx, sy) 取周围 2x2 四个点加权
 * 权重：与 (sx,sy) 的水平/垂直距离，越近权重越大
 * 设 fx = sx - floor(sx), fy = sy - floor(sy)
 * 则 p00*(1-fx)*(1-fy) + p10*fx*(1-fy) + p01*(1-fx)*fy + p11*fx*fy
 */
inline float sample_bilinear(const float* src, int src_w, int src_h, int channels,
                             double sx, double sy, int c) {
    const int x0 = static_cast<int>(std::floor(sx));
    const int y0 = static_cast<int>(std::floor(sy));
    const double fx = sx - x0;
    const double fy = sy - y0;

    /* 边界钳位：防止越界 */
    const int x1 = std::min(x0 + 1, src_w - 1);
    const int y1 = std::min(y0 + 1, src_h - 1);
    const int x0c = std::max(0, x0);
    const int y0c = std::max(0, y0);

    const int stride = src_w * channels;
    const float p00 = src[y0c * stride + x0c * channels + c];
    const float p10 = src[y0c * stride + x1 * channels + c];
    const float p01 = src[y1 * stride + x0c * channels + c];
    const float p11 = src[y1 * stride + x1 * channels + c];

    const float v0 = static_cast<float>((1.0 - fx) * p00 + fx * p10);
    const float v1 = static_cast<float>((1.0 - fx) * p01 + fx * p11);
    return static_cast<float>((1.0 - fy) * v0 + fy * v1);
}

} // anonymous namespace

bool resize_bilinear(const ImageBuf& src, ImageBuf& dst,
                     int dst_width, int dst_height, int dst_channels,
                     ResizePerfPoint* out_perf) {
    if (src.empty() || dst_width <= 0 || dst_height <= 0) {
        return false;
    }
    const int ch = (dst_channels > 0) ? dst_channels : src.channels;
    if (ch != src.channels) {
        return false;
    }

    double scale_x, scale_y;
    scale_factors(src.width, src.height, dst_width, dst_height, scale_x, scale_y);

    dst.width    = dst_width;
    dst.height   = dst_height;
    dst.channels = ch;
    dst.data.resize(static_cast<std::size_t>(dst_width) * dst_height * ch);

    const float* s = src.data.data();
    float* d = dst.data.data();
    const int src_w = src.width;
    const int src_h = src.height;
    const int stride_out = dst_width * ch;

    /* 性能打点：仅测量主循环耗时（不含分配等开销） */
    auto t0 = std::chrono::steady_clock::now();

    /*
     * 三重循环的含义：
     *   dy, dx：目标图上的像素网格坐标 (x, y)
     *   c     ：通道索引（0..channels-1）
     *
     *   1) 先根据 (dy, dx) 计算出源图上的浮点坐标 (sy, sx)，采用“像素中心对齐”的写法：
     *        sy = (dy + 0.5) * scale_y - 0.5
     *        sx = (dx + 0.5) * scale_x - 0.5
     *      这样当 src/dst 尺寸相同时，源 / 目标像素中心能很好对齐。
     *
     *   2) 对于每个通道 c，调用 sample_bilinear(sx, sy, c)，在源图上取 2x2 邻域做双线性插值；
     *      返回的就是“源图在 (sx, sy) 位置、第 c 通道”的估计值。
     *
     *   3) 写回目标缓冲区时，使用行优先的交错布局：
     *        index = dy * stride_out + dx * channels + c
     *      其中 stride_out = dst_width * channels，相当于一行的元素总数。
     *
     *   4) 外层 dy 循环在启用 OpenMP 时可以安全并行，因为每个输出像素写入的是
     *      不同位置，且只读 src.data，不存在数据竞争。
     */

#ifdef IMGRESIZE_USE_OPENMP
    #pragma omp parallel for
#endif
    for (int dy = 0; dy < dst_height; ++dy) {
        const double sy = (dy + 0.5) * scale_y - 0.5;  // 输出行 dy 对应的源图浮点 y
        for (int dx = 0; dx < dst_width; ++dx) {
            const double sx = (dx + 0.5) * scale_x - 0.5;  // 输出列 dx 对应的源图浮点 x
            for (int c = 0; c < ch; ++c) {
                d[dy * stride_out + dx * ch + c] = sample_bilinear(
                    s, src_w, src_h, ch, sx, sy, c);
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    double time_ms = std::chrono::duration<double, std::chrono::milliseconds::period>(t1 - t0).count();

    /* 性能打点：若调用方传入 out_perf，则回填本次 resize 的耗时与吞吐 */
    if (out_perf) {
        out_perf->src_w = src.width;
        out_perf->src_h = src.height;
        out_perf->dst_w = dst_width;
        out_perf->dst_h = dst_height;
        out_perf->channels = ch;
        out_perf->time_ms = time_ms;
        const double out_mpix = (dst_width * dst_height) / 1e6;
        out_perf->megapixels_per_sec = (time_ms > 0) ? (out_mpix / (time_ms / 1000.0)) : 0.0;
    }

    return true;
}

} // namespace imgresize
