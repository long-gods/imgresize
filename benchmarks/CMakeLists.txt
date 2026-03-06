# 基准测试：多组尺寸性能打点并生成报告
add_executable(bench_resize bench_resize.cpp)
target_link_libraries(bench_resize PRIVATE imgresize)
target_include_directories(bench_resize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
if(MSVC)
    target_compile_options(bench_resize PRIVATE /utf-8)
endif()

# 报告输出目录（可选）
set(REPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/reports" CACHE PATH "Bench report output")
