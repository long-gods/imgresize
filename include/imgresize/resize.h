#pragma once

#include "types.h"
#include "perf_report.h"

namespace imgresize {

/**
 * 双线性插值缩放（带可选性能打点）
 *
 * 将 src 缩放到 dst 的尺寸，采用双线性插值 (Bilinear Interpolation)。
 * 坐标映射：输出像素 (dx, dy) 对应源图浮点坐标 (sx, sy)，
 * 取周围 2x2 四个源像素加权平均得到输出像素值。
 *
 * @param src  源图像（只读）
 * @param dst  目标图像（由本函数按 dst_width/dst_height/dst_channels 分配并写入）
 * @param dst_width  目标宽度
 * @param dst_height 目标高度
 * @param dst_channels 目标通道数，与 src.channels 一致时可省略传 0
 * @param out_perf 若非空，则填入本次 resize 的耗时等性能打点数据（见 perf_report.h）
 * @return 是否成功；失败时 dst 可能未修改
 */
bool resize_bilinear(const ImageBuf& src,
                     ImageBuf& dst,
                     int dst_width,
                     int dst_height,
                     int dst_channels = 0,
                     ResizePerfPoint* out_perf = nullptr);

} // namespace imgresize
