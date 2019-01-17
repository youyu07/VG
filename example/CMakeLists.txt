cmake_minimum_required(VERSION 3.6)

project(example)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(example WIN32 ${EXAMPLE_SOURCE})
target_include_directories(example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../vg)
target_link_libraries(example PRIVATE vg)
#add_dependencies(example vg)