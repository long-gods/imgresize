#pragma once

#include <cstddef>
#include <vector>

namespace imgresize {

/**
 * 简单图像缓冲区描述
 * 内存布局：行优先 (row-major)，即 data[y * width * channels + x * channels + c]
 * channels: 1=灰度, 3=RGB, 4=RGBA
 */
struct ImageBuf {
    int width   = 0;
    int height  = 0;
    int channels = 1;
    std::vector<float> data;

    std::size_t size_bytes() const {
        return data.size() * sizeof(float);
    }

    bool empty() const {
        return data.empty() || width <= 0 || height <= 0 || channels <= 0;
    }

    /** 像素总数 (width * height * channels) */
    int pixel_count() const {
        return width * height * channels;
    }
};

} // namespace imgresize
