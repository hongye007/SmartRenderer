# Source files for SmartRenderer library

# Core source files
set(CORE_SOURCES
    core/RenderGraph.cpp
    core/RenderNode.cpp
    core/RenderProtocol.cpp
    core/Renderer.cpp
    SmartRenderer.cpp
)

# Math library sources
set(MATH_SOURCES
    math/Vector.cpp
    math/Matrix.cpp
    math/Quaternion.cpp
    math/MathUtils.cpp
    math/Color.cpp
)

# Platform sources (will be added conditionally)
set(PLATFORM_SOURCES
    platform/Platform.cpp
)

# Rendering sources
set(RENDERING_SOURCES
    rendering/opengles/OpenGLESRenderer.cpp
    rendering/opengles/OpenGLESResourceFactory.cpp
    rendering/nodes/GeometryRenderNode.cpp
    rendering/nodes/PostProcessNode.cpp
    rendering/nodes/ShadowRenderNode.cpp
    rendering/nodes/SkyboxNode.cpp
)

# Resource management sources
set(RESOURCE_SOURCES
    resource/ResourceManager.cpp
    resource/AssetLoader.cpp
)

# Remove duplicate OpenGL ES shader/texture/buffer files (using factory instead)
# Keep only the factory implementation

# Platform-specific sources
if(PLATFORM_WINDOWS)
    list(APPEND PLATFORM_SOURCES
        platform/windows/WindowsPlatform.cpp
        platform/windows/WindowsWindow.cpp
        platform/windows/WindowsInputManager.cpp
        platform/windows/WindowsFileSystem.cpp
    )
elseif(PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        platform/macos/MacOSPlatform.cpp
    )
elseif(PLATFORM_IOS)
    list(APPEND PLATFORM_SOURCES
        platform/ios/IOSPlatform.cpp
    )
elseif(PLATFORM_ANDROID)
    list(APPEND PLATFORM_SOURCES
        platform/android/AndroidPlatform.cpp
    )
endif()

# ANGLE integration
if(USE_ANGLE)
    list(APPEND RENDERING_SOURCES
        rendering/angle/ANGLEContext.cpp
        rendering/angle/ANGLEConfig.cpp
    )
endif()

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${MATH_SOURCES}
    ${PLATFORM_SOURCES}
    ${RENDERING_SOURCES}
    ${RESOURCE_SOURCES}
)

# Create library
add_library(SmartRenderer ${ALL_SOURCES})

# Target properties
target_include_directories(SmartRenderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(SmartRenderer
    PRIVATE
)

# Platform-specific linking
if(PLATFORM_WINDOWS)
    # Windows-specific libraries (only if not using ANGLE)
    if(NOT USE_ANGLE)
        target_link_libraries(SmartRenderer PRIVATE opengl32)
    endif()
elseif(PLATFORM_MACOS OR PLATFORM_IOS)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    target_link_libraries(SmartRenderer PRIVATE
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
    )
elseif(PLATFORM_ANDROID)
    find_library(ANDROID_LOG_LIB log)
    target_link_libraries(SmartRenderer PRIVATE ${ANDROID_LOG_LIB})
endif()

# ANGLE integration
if(USE_ANGLE)
    message(STATUS "ANGLE integration enabled")
    
    # Find ANGLE libraries
    set(ANGLE_ROOT "${CMAKE_SOURCE_DIR}/third_party/angle")
    set(ANGLE_INCLUDE_DIR "${ANGLE_ROOT}/include")
    set(ANGLE_LIB_DIR "${ANGLE_ROOT}/lib")
    
    if(EXISTS "${ANGLE_INCLUDE_DIR}/EGL/egl.h")
        message(STATUS "Found ANGLE headers: ${ANGLE_INCLUDE_DIR}")
        target_include_directories(SmartRenderer PUBLIC ${ANGLE_INCLUDE_DIR})
        
        # Link ANGLE libraries
        if(PLATFORM_WINDOWS)
            find_library(ANGLE_EGL_LIB NAMES libEGL PATHS ${ANGLE_LIB_DIR} NO_DEFAULT_PATH)
            find_library(ANGLE_GLES_LIB NAMES libGLESv2 PATHS ${ANGLE_LIB_DIR} NO_DEFAULT_PATH)
            
            if(ANGLE_EGL_LIB AND ANGLE_GLES_LIB)
                message(STATUS "Found ANGLE EGL: ${ANGLE_EGL_LIB}")
                message(STATUS "Found ANGLE GLESv2: ${ANGLE_GLES_LIB}")
                target_link_libraries(SmartRenderer PRIVATE ${ANGLE_EGL_LIB} ${ANGLE_GLES_LIB})
                
                # Add preprocessor definition
                target_compile_definitions(SmartRenderer PUBLIC USE_ANGLE=1)
            else()
                message(WARNING "ANGLE libraries not found in ${ANGLE_LIB_DIR}")
            endif()
        endif()
    else()
        message(WARNING "ANGLE headers not found. Please run third_party/setup_angle.ps1")
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(SmartRenderer PRIVATE /W4 /WX-)
else()
    target_compile_options(SmartRenderer PRIVATE -Wall -Wextra -Wpedantic)
endif()
