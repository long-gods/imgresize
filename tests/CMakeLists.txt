# 单元测试（可选用 CTest 或独立可执行文件）
add_executable(test_resize test_resize.cpp)
target_link_libraries(test_resize PRIVATE imgresize)
target_include_directories(test_resize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
if(MSVC)
    target_compile_options(test_resize PRIVATE /utf-8)
endif()
