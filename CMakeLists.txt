cmake_minimum_required(VERSION 3.16)
project(minbpe LANGUAGES CXX)

# 要求至少 C++17（使用 structured bindings, auto& [a,b] 等特性）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 跨平台启用编译警告
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 源文件路径
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 构建可执行文件
add_executable(minbpe
    ${SRC_DIR}/main.cpp
)

# 让编译器知道头文件路径
target_include_directories(minbpe PRIVATE ${SRC_DIR})

# 可执行文件输出目录
set_target_properties(minbpe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 打印信息（调试友好）
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source dir: ${SRC_DIR}")
