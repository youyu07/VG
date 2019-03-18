cmake_minimum_required(VERSION 3.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(vg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Vulkan_DIR "$ENV{VULKAN_SDK}")
include_directories(${Vulkan_DIR}/Third-Party/Include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(Shaderc_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaderc)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_subdirectory(vg)
add_subdirectory(example)