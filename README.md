# imgresize — 图像缩放库

*本文件为 UTF-8 编码。*

## 项目介绍

**imgresize** 是一个轻量级 C++ 图像缩放库，提供基于**双线性插值（Bilinear Interpolation）**的 `resize` 实现。适用于需要可读、可维护、且带性能分析能力的缩放逻辑的场景。

### 特性

- **双线性插值**：输出像素由源图 2×2 邻域加权平均得到，在质量与速度之间取得平衡。
- **可读实现**：核心逻辑带详细注释，便于学习与二次开发。
- **性能打点**：`resize_bilinear` 支持可选性能数据回填（耗时、吞吐），便于集成到性能分析报告。
- **无第三方依赖**：仅依赖 C++17 标准库，易于集成到现有工程。

### 目录结构

```
imgresize/  (或项目所在目录名，如 picsuofang/)
├── CMakeLists.txt
├── README.md
├── include/
│   └── imgresize/
│       ├── types.h        # 图像缓冲区类型 ImageBuf
│       ├── resize.h       # 双线性缩放接口
│       └── perf_report.h  # 性能打点与报告接口
├── src/
│   ├── resize.cpp        # 双线性插值实现
│   └── perf_report.cpp   # 报告生成与写文件
├── tests/
│   ├── CMakeLists.txt
│   └── test_resize.cpp   # 单元测试
└── benchmarks/
    ├── CMakeLists.txt
    └── bench_resize.cpp  # 基准测试与性能报告生成
```

---

## 性能对比

以下为**示例性能数据**（Release 构建，单线程，典型桌面环境），实际数值随 CPU、内存与分辨率变化。

| 源尺寸     | 目标尺寸    | 通道 | 耗时 (ms) | 吞吐 (Mpix/s) |
|------------|-------------|------|-----------|----------------|
| 640×480    | 320×240     | 3    | ~0.5      | ~46            |
| 1920×1080  | 960×540     | 3    | ~3.5      | ~159           |
| 1920×1080  | 1920×1080   | 3    | ~12       | ~173           |
| 3840×2160  | 1920×1080   | 3    | ~15       | ~265           |

- **Mpix/s**：每秒输出百万像素数，越大表示缩放吞吐越高。
- 运行 `bench_resize` 可在本机生成完整性能报告（控制台输出 + 可选报告文件）。

---

## 使用方法

### 构建

在项目根目录下：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 运行测试与基准

```bash
# 单元测试
./build/Release/tests/test_resize.exe    # Windows
./build/tests/test_resize                # Linux/macOS

# 基准测试（打印性能表并生成报告）
./build/Release/benchmarks/bench_resize.exe
# 报告默认写入 reports/resize_perf_report.txt（需存在 reports 目录或由程序尝试创建）
```

### 在代码中调用

```cpp
#include "imgresize/resize.h"
#include "imgresize/types.h"

imgresize::ImageBuf src;
src.width = 1920;
src.height = 1080;
src.channels = 3;
src.data.resize(src.width * src.height * src.channels);
// ... 填充 src.data ...

imgresize::ImageBuf dst;
imgresize::ResizePerfPoint perf;  // 可选：性能打点
bool ok = imgresize::resize_bilinear(src, dst, 960, 540, 3, &perf);
if (ok) {
    // 使用 dst.width, dst.height, dst.data
    // 若传了 &perf，可读取 perf.time_ms、perf.megapixels_per_sec 等
}
```

### 性能打点与报告

- 调用 `resize_bilinear(..., &perf)` 时，函数会填充 `ResizePerfPoint`（耗时、源/目标尺寸、吞吐等）。
- 使用 `imgresize::generate_perf_report(points, title)` 可生成表格字符串。
- 使用 `imgresize::write_perf_report_to_file(path, points, title)` 可将报告写入文件（会尝试创建父目录）。

---

## 许可证与贡献

本项目为示例/教学用途。可根据需要自行修改或集成到产品中。如有问题或改进建议，欢迎提 Issue 或 PR。
