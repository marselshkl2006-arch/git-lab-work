cmake_minimum_required(VERSION 3.16)
project(ChemicalLabQt VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/database.cpp
    src/mainwindow_full.cpp
    src/dialogs.cpp
)

set(HEADERS
    src/mainwindow.h
    src/database.h
    src/dialogs.h
)

add_executable(ChemicalLabQt ${SOURCES} ${HEADERS})

target_link_libraries(ChemicalLabQt
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(ChemicalLabQt PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ChemicalLabQt PRIVATE -Wall -Wextra)
endif()
