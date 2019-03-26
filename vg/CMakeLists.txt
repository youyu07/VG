find_package(Vulkan REQUIRED)


set(VG_SOURCE
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_win32.cpp
	render/context.cpp
	render/renderer.cpp
	util/entry.cpp
	util/entry_win.cpp
	util/geometry.cpp)

add_library(vg SHARED ${VG_SOURCE})

add_definitions(-DNOMINMAX)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
add_definitions(-DVKU_SURFACE="VK_KHR_win32_surface")

target_include_directories(vg PRIVATE Vulkan::Vulkan ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vg PRIVATE Vulkan::Vulkan ${Shaderc_DIR}/lib/shaderc_shared.lib)

add_custom_command(TARGET vg POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Shaderc_DIR}/bin/shaderc_shared.dll"
        $<TARGET_FILE_DIR:vg>)

install(TARGETS vg RUNTIME DESTINATION bin)
install(FILES "${Shaderc_DIR}/bin/shaderc_shared.dll" RUNTIME DESTINATION bin)