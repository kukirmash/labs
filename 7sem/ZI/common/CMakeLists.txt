# Создаем статическую библиотеку common_lib
add_library(common_lib utils.cpp)

# Указываем директорию с заголовочными файлами (.h)
# PUBLIC означает, что пути include будут переданы проектам, использующим common_lib
target_include_directories(common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})