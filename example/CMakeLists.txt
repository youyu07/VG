cmake_minimum_required(VERSION 3.6)

project(example)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(example WIN32 ${EXAMPLE_SOURCE})

add_dependencies(example vgcore vgutil)