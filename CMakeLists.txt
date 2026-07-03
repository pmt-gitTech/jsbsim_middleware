cmake_minimum_required(VERSION 3.10)

project(Middleware)

set(CMAKE_CXX_STANDARD 17)

add_executable(
    JSBSim_Middleware
    JSBSim_Middleware_main.cpp
)

target_link_libraries(
    JSBSim_Middleware
    ws2_32
)