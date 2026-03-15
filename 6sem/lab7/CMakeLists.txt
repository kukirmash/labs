# Версия CMake
cmake_minimum_required(VERSION 3.10)

# Убираем предупреждение про OpenGL
cmake_policy(SET CMP0072 NEW)

# Имя проекта
project(OpenGLProject_lab7)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исполняемый файл
add_executable(main 
    # ------MAIN------
    src/main.cpp 
    
    # ---Шейдеры---
    src/Shader/Shader.cpp

    # ---Счётчик FPS---
    src/FPSCounter/FPSCounter.cpp

    # ---GraphicObject---
    src/GraphicObject/GraphicObject.cpp

    # ---Camera---
    src/Camera/Camera.cpp

    # ---Ввод---
    src/InputControl/InputControl.cpp

    # ---Менеджер ресурсов---
    src/ResourceManager/ResourceManager.cpp
    
    # ---Меш---
    src/Mesh/Mesh.cpp
    
    # ---Текстуры---
    src/Texture/Texture.cpp

    # ---Рендер Менеджер---
    src/RenderManager/RenderManager.cpp

    src/Material/Material.cpp

    src/Scene/Scene.cpp
)

# ИЩЕМ библиотеки
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)
find_package(DevIL REQUIRED)

# Подключаем найденные библиотеки
target_link_libraries(main
    OpenGL::GL
    GLEW::GLEW
    GLUT::GLUT
    glm::glm
    IL          
)

# Добавляем текущую папку для заголовочных файлов проекта
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DevIL_INCLUDE_DIR}
)

# Автоматически копируем шейдеры и ресурсы в папку со сборкой (build)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/DATA/MESHES DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/DATA/TEXTURES DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/DATA/SHADERS DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/DATA/MATERIALS DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/DATA/SCENES DESTINATION ${CMAKE_CURRENT_BINARY_DIR})