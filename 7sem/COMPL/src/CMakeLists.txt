cmake_minimum_required(VERSION 3.16)
project(scanner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp

    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/CodeEditor.cpp
    ui/CodeEditor.h

    scanner/Scanner.cpp
    scanner/Scanner.h
    scanner/Token.cpp
    scanner/Token.h
    scanner/TableId_Elem.h

    sut/LRTable.cpp
    sut/LRTable.h
    sut/SymbolTable.cpp
    sut/SymbolTable.h
    sut/Tac.cpp
    sut/Tac.h
    sut/SemanticActions.cpp
    sut/SemanticActions.h
    sut/SutParser.cpp
    sut/SutParser.h
    sut/SutTypes.h

    third_party/pugixml/pugixml.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/sut
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pugixml
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
