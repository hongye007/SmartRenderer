cmake_minimum_required(VERSION 3.16)
project(SmartRenderer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC: avoid PDB contention when building multiple translation units
if(MSVC)
    # /FS allows concurrent PDB writes, /Z7 embeds debug info in .obj to reduce PDB contention
    add_compile_options(/FS /Z7)
endif()

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS ON)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    if(IOS)
        set(PLATFORM_IOS ON)
        message(STATUS "Platform: iOS")
    else()
        set(PLATFORM_MACOS ON)
        message(STATUS "Platform: macOS")
    endif()
elseif(ANDROID)
    set(PLATFORM_ANDROID ON)
    message(STATUS "Platform: Android")
elseif(UNIX)
    set(PLATFORM_LINUX ON)
    message(STATUS "Platform: Linux")
endif()

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_SAMPLES "Build sample applications" ON)
option(USE_ANGLE "Use ANGLE for rendering backend" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Add subdirectories
add_subdirectory(src)

# Add samples if enabled
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# Installation
install(TARGETS SmartRenderer
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
