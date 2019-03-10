find_package(Vulkan REQUIRED)


set(Vulkan_DIR "$ENV{VULKAN_SDK}")


set(VG_SOURCE
    render/device.cpp
	render/renderer.cpp
	render/shader.cpp
	util/entry.cpp
	util/entry_win.cpp)

add_library(vg SHARED ${VG_SOURCE})

target_include_directories(vg PRIVATE Vulkan::Vulkan ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vg PRIVATE Vulkan::Vulkan ${Shaderc_DIR}/lib/shaderc_shared.lib)

add_custom_command(TARGET vg POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Shaderc_DIR}/bin/shaderc_shared.dll"
        $<TARGET_FILE_DIR:vg>)

install(TARGETS vg RUNTIME DESTINATION bin)
install(FILES "${Shaderc_DIR}/bin/shaderc_shared.dll" RUNTIME DESTINATION bin)