set(EXAMPLE_SOURCE main.cpp)

add_executable(example WIN32 ${EXAMPLE_SOURCE})
target_include_directories(example PRIVATE ../vg)
target_link_libraries(example PRIVATE vg)
add_dependencies(example vg)

install(TARGETS example RUNTIME DESTINATION bin)