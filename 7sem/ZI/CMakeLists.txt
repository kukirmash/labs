cmake_minimum_required(VERSION 3.10)
project(ZI_Workspace)

# Сначала собираем общую библиотеку
add_subdirectory(common)

# Затем подключаем проекты, которым нужна эта библиотека
add_subdirectory(lab1)
add_subdirectory(lab2)
add_subdirectory(lab3)
