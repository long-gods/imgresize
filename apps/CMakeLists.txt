# 命令行工具：resize_png input.png output.png w h
add_executable(resize_png
    resize_png.cpp
)

target_link_libraries(resize_png
    PRIVATE
        imgresize
)

target_include_directories(resize_png
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(MSVC)
    target_link_libraries(resize_png PRIVATE Gdiplus)
    target_compile_options(resize_png PRIVATE /utf-8)
endif()

