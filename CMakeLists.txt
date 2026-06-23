cmake_minimum_required(VERSION 3.15)
project(StackyNG VERSION 0.7.0.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)

set(SOURCES
    src/stacky.cpp src/app.cpp src/cache_manager.cpp src/bmp.cpp 
    src/config.cpp src/util.cpp src/logger.cpp src/stacky.rc src/resource.h
)

if(MSVC)
    add_executable(stacky WIN32 ${SOURCES})
    target_link_options(stacky PRIVATE "/SUBSYSTEM:WINDOWS")
else()
    add_executable(stacky ${SOURCES})
endif()

target_link_libraries(stacky PRIVATE
    user32 gdi32 shell32 ole32 comctl32 windowscodecs advapi32 uxtheme dwmapi
)
