cmake_minimum_required(VERSION 3.6)

project(vg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# core
set(VG_CORE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/core/instance.h)

add_library(vgcore SHARED ${VG_CORE_SOURCE})

target_include_directories(vgcore PRIVATE Vulkan::Vulkan)
target_link_libraries(vgcore PRIVATE Vulkan::Vulkan)


# util
set(VG_UTIL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/util/entry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/entry.cpp)

add_library(vgutil ${VG_CORE_SOURCE})

target_include_directories(vgutil PRIVATE Vulkan::Vulkan)
target_link_libraries(vgutil PRIVATE Vulkan::Vulkan)


IF(MSVC)
    # make it looks better in msvc
    foreach(source IN LISTS VG_CORE_SOURCE VG_UTIL_SOURCE)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
ENDIF(MSVC)