cmake_minimum_required(VERSION 3.6)

project(vg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# core
set(VG_CORE_SOURCE
    core/vulkan/device.cpp)

set(VG_UTIL_SOURCE
    util/entry.cpp
	util/entry_win.cpp)

add_library(vg SHARED vg.cpp ${VG_CORE_SOURCE} ${VG_UTIL_SOURCE})

target_include_directories(vg PRIVATE Vulkan::Vulkan ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vg PRIVATE Vulkan::Vulkan)