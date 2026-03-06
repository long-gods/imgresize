cmake_minimum_required(VERSION 3.15)

project(imgresize
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

option(IMGRESIZE_ENABLE_OPENMP "Enable OpenMP for imgresize" ON)

# 图像缩放库（静态库）
add_library(imgresize STATIC
    src/resize.cpp
    src/perf_report.cpp
)
target_include_directories(imgresize PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(IMGRESIZE_ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        message(STATUS "[imgresize] OpenMP found, enabling for C++")
        target_link_libraries(imgresize PRIVATE OpenMP::OpenMP_CXX)
        target_compile_definitions(imgresize PRIVATE IMGRESIZE_USE_OPENMP)
    else()
        message(WARNING "[imgresize] OpenMP not found, building without OpenMP")
    endif()
endif()

# 性能报告输出目录（可选）
set(IMGRESIZE_REPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/reports" CACHE PATH "Performance report output dir")

# 单元测试
add_subdirectory(tests)

# 基准测试
add_subdirectory(benchmarks)

# 示例应用（命令行工具）
add_subdirectory(apps)

if(MSVC)
    target_compile_options(imgresize PRIVATE /utf-8)
endif()
