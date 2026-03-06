/**
 * @file test_resize.cpp
 * @brief 双线性插值 resize 的单元测试
 */

#include "imgresize/resize.h"
#include "imgresize/types.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace imgresize;

static int passed = 0, failed = 0;

#define ASSERT(cond) do { \
    if (!(cond)) { std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; ++failed; } \
    else { ++passed; } \
} while(0)

/** 生成简单渐变图：data[y*w*c + x*c + ch] = (x+y)/(w+h) */
static ImageBuf make_gradient(int w, int h, int c) {
    ImageBuf img;
    img.width = w;
    img.height = h;
    img.channels = c;
    img.data.resize(static_cast<std::size_t>(w) * h * c);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            for (int ch = 0; ch < c; ++ch)
                img.data[static_cast<std::size_t>(y) * w * c + x * c + ch] =
                    static_cast<float>(x + y) / (w + h);
    return img;
}

int main() {
    /* 1. 空源图应失败 */
    ImageBuf empty;
    ImageBuf out;
    ASSERT(!resize_bilinear(empty, out, 10, 10));

    /* 2. 合法缩放应成功，尺寸正确 */
    ImageBuf src = make_gradient(100, 100, 1);
    ASSERT(resize_bilinear(src, out, 50, 50));
    ASSERT(out.width == 50 && out.height == 50 && out.channels == 1);
    ASSERT(out.data.size() == static_cast<std::size_t>(50 * 50 * 1));

    /* 3. 放大 */
    ASSERT(resize_bilinear(src, out, 200, 200));
    ASSERT(out.width == 200 && out.height == 200);

    /* 4. 同尺寸（应得到近似相同值） */
    ImageBuf same;
    ASSERT(resize_bilinear(src, same, 100, 100));
    ASSERT(same.width == 100 && same.height == 100);
    float max_diff = 0.f;
    for (std::size_t i = 0; i < src.data.size(); ++i)
        max_diff = std::max(max_diff, std::fabs(src.data[i] - same.data[i]));
    ASSERT(max_diff < 1e-5f);

    /* 5. 3 通道 */
    ImageBuf src3 = make_gradient(64, 64, 3);
    ASSERT(resize_bilinear(src3, out, 32, 32, 3));
    ASSERT(out.channels == 3 && out.data.size() == static_cast<std::size_t>(32 * 32 * 3));

    /* 6. 性能打点可选参数：不传应正常 */
    ASSERT(resize_bilinear(src, out, 80, 80, 0, nullptr));

    /* 7. 传入 out_perf 应被填充 */
    ResizePerfPoint perf;
    ASSERT(resize_bilinear(src, out, 40, 40, 0, &perf));
    ASSERT(perf.src_w == 100 && perf.src_h == 100 && perf.dst_w == 40 && perf.dst_h == 40);
    ASSERT(perf.time_ms >= 0 && perf.megapixels_per_sec >= 0);

    std::cout << "Tests: " << passed << " passed, " << failed << " failed.\n";
    return failed ? 1 : 0;
}
