cmake_minimum_required(VERSION 3.10.0)

project(lab3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab3 main.cpp)

target_link_libraries(lab3 common_lib)
