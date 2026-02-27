# Версия CMake
cmake_minimum_required(VERSION 3.10)

# Имя проекта
project(OpenGLProject_lab2)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исполняемый файл
add_executable(main 
    # ------MAIN------
    main.cpp 
    
    # ---Шейдеры---
    Shader/Shader.cpp

    # ---Счётчик FPS---
    FPSCounter/FPSCounter.cpp
)

# ИЩЕМ библиотеки
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

# Подключаем найденные библиотеки
target_link_libraries(main
    OpenGL::GL
    GLEW::GLEW
    GLUT::GLUT
    glm::glm
)

# Добавляем текущую папку для заголовочных файлов проекта
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)