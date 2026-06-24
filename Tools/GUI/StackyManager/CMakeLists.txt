cmake_minimum_required(VERSION 3.20)
project(StackyManager VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

# Define the Executable
add_executable(StackyManager
    src/main.cpp
    src/TrayIcon.cpp
    src/ShellManager.cpp
    src/NotificationManager.cpp
    src/Utils.cpp
    resources/StackyManager.rc
)

# Link necessary Windows system libraries
target_link_libraries(StackyManager PRIVATE
    shell32.lib
    ole32.lib
    uuid.lib
	UxTheme.lib
    dwmapi.lib
    propsys.lib
    runtimeobject.lib
)

target_include_directories(StackyManager PRIVATE include)

target_sources(StackyManager PRIVATE app.manifest)
set_property(SOURCE app.manifest PROPERTY MACOSX_PACKAGE_LOCATION "Resources")

# --- Configuration-specific settings ---

# Add definitions for Debug/Release
target_compile_definitions(StackyManager PRIVATE
    $<$<CONFIG:Debug>:STACKY_DEBUG=1>
    $<$<CONFIG:Release>:STACKY_DEBUG=0>
)

# Set output names: StackyManager_d.exe for Debug, StackyManager.exe for Release
set_target_properties(StackyManager PROPERTIES
	WIN32_EXECUTABLE TRUE
    DEBUG_POSTFIX "_debug"
	LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\""
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Optional: Add Windows Subsystem flag for Release (no console window)
target_link_options(StackyManager PRIVATE 
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
)

if(MSVC)
    # Corrected: Pass flags as individual elements in the list
    target_link_options(StackyManager PRIVATE 
        "/SUBSYSTEM:WINDOWS" 
        "/ENTRY:wWinMainCRTStartup"
    )
endif()
