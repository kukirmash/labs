cmake_minimum_required(VERSION 3.16)
project(compl_scanner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Указываем файлы с учетом их новых папок и названий CamelCase
add_executable(${PROJECT_NAME}
    main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/CodeEditor.cpp  
    ui/CodeEditor.h
)

# Говорим компилятору искать заголовочные файлы (.h) в этих папках
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/ui 
    ${CMAKE_CURRENT_SOURCE_DIR}/scanner
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)