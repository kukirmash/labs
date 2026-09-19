cmake_minimum_required(VERSION 3.10.0)

project(lab1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab1 main.cpp)

# Подключаем общую библиотеку к проекту
target_link_libraries(lab1 common_lib)