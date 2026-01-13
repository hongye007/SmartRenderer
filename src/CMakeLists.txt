# Source files for SmartRenderer library

# Core source files
set(CORE_SOURCES
    core/RenderGraph.cpp
    core/RenderNode.cpp
    core/RenderProtocol.cpp
    core/Renderer.cpp
    core/RenderCommand.cpp
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
    resource/ResourceRegistry.cpp
    resource/ResourceFactory.cpp
)

# ECS sources
set(ECS_SOURCES
    ecs/World.cpp
    ecs/components/Transform.cpp
    ecs/components/Camera.cpp
    ecs/systems/RenderSystem.cpp
    ecs/ECSProtocol.cpp
)

# Remove duplicate OpenGL ES shader/texture/buffer files (using factory instead)
# Keep only the factory implementation

# Platform-specific sources
# GLFWWindowBase is shared by Windows and macOS
if(PLATFORM_WINDOWS OR PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        platform/GLFWWindowBase.cpp
    )
endif()

if(PLATFORM_WINDOWS)
    list(APPEND PLATFORM_SOURCES
        platform/windows/WindowsPlatform.cpp
        platform/windows/WindowsWindow.cpp
    )
elseif(PLATFORM_MACOS)
    list(APPEND PLATFORM_SOURCES
        platform/macos/MacOSPlatform.cpp
        platform/macos/MacOSWindow.cpp
    )
    # Set macOS files to compile as Objective-C++
    set_source_files_properties(platform/macos/MacOSPlatform.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
    set_source_files_properties(platform/macos/MacOSWindow.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
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
    ${ECS_SOURCES}
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
    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    target_link_libraries(SmartRenderer PRIVATE
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${OPENGL_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
elseif(PLATFORM_ANDROID)
    find_library(ANDROID_LOG_LIB log)
    target_link_libraries(SmartRenderer PRIVATE ${ANDROID_LOG_LIB})
endif()

# GLFW integration for macOS and Windows only
# Android and iOS use native windowing APIs
if(PLATFORM_MACOS)
    set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/third_party/gl-env/glfw/mac")
    set(GLFW_INCLUDE_DIR "${GLFW_ROOT}/include")
    set(GLFW_LIB_DIR "${GLFW_ROOT}/lib-universal")
    
    if(EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")
        message(STATUS "Found GLFW headers: ${GLFW_INCLUDE_DIR}")
        target_include_directories(SmartRenderer PUBLIC ${GLFW_INCLUDE_DIR})
        
        # Link GLFW library
        set(GLFW_LIB "${GLFW_LIB_DIR}/libglfw3.a")
        if(EXISTS "${GLFW_LIB}")
            message(STATUS "Found GLFW library: ${GLFW_LIB}")
            target_link_libraries(SmartRenderer PRIVATE ${GLFW_LIB})
        else()
            message(WARNING "GLFW library not found at ${GLFW_LIB}")
        endif()
    else()
        message(WARNING "GLFW headers not found at ${GLFW_INCLUDE_DIR}")
    endif()
elseif(PLATFORM_WINDOWS)
    set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/third_party/gl-env/glfw/windows")
    set(GLFW_INCLUDE_DIR "${GLFW_ROOT}/include")
    set(GLFW_LIB_DIR "${GLFW_ROOT}/lib")
    
    if(EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")
        message(STATUS "Found GLFW headers: ${GLFW_INCLUDE_DIR}")
        target_include_directories(SmartRenderer PUBLIC ${GLFW_INCLUDE_DIR})
        
        # Link GLFW library (Windows uses .lib)
        set(GLFW_LIB "${GLFW_LIB_DIR}/glfw3.lib")
        if(EXISTS "${GLFW_LIB}")
            message(STATUS "Found GLFW library: ${GLFW_LIB}")
            target_link_libraries(SmartRenderer PRIVATE ${GLFW_LIB})
        else()
            message(WARNING "GLFW library not found at ${GLFW_LIB}")
        endif()
    else()
        message(WARNING "GLFW headers not found at ${GLFW_INCLUDE_DIR}")
    endif()
endif()

# ANGLE integration
if(USE_ANGLE)
    message(STATUS "ANGLE integration enabled")
    
    # Find ANGLE libraries from gl-env directory
    set(ANGLE_ROOT "${CMAKE_SOURCE_DIR}/third_party/gl-env/angle")
    set(ANGLE_INCLUDE_DIR "${ANGLE_ROOT}/include")
    
    if(EXISTS "${ANGLE_INCLUDE_DIR}/EGL/egl.h")
        message(STATUS "Found ANGLE headers: ${ANGLE_INCLUDE_DIR}")
        target_include_directories(SmartRenderer PUBLIC ${ANGLE_INCLUDE_DIR})
        
        # Link ANGLE libraries based on platform
        if(PLATFORM_WINDOWS)
            set(ANGLE_LIB_DIR "${ANGLE_ROOT}/libs/windows")
            
            # Windows: Use .dll.lib import libraries
            set(ANGLE_EGL_LIB "${ANGLE_LIB_DIR}/libEGL.dll.lib")
            set(ANGLE_GLES_LIB "${ANGLE_LIB_DIR}/libGLESv2.dll.lib")
            
            if(EXISTS "${ANGLE_EGL_LIB}" AND EXISTS "${ANGLE_GLES_LIB}")
                message(STATUS "Found ANGLE EGL: ${ANGLE_EGL_LIB}")
                message(STATUS "Found ANGLE GLESv2: ${ANGLE_GLES_LIB}")
                target_link_libraries(SmartRenderer PRIVATE ${ANGLE_EGL_LIB} ${ANGLE_GLES_LIB})
                
                # Copy DLLs to output directory
                add_custom_command(TARGET SmartRenderer POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${ANGLE_LIB_DIR}/libEGL.dll"
                        "${ANGLE_LIB_DIR}/libGLESv2.dll"
                        $<TARGET_FILE_DIR:SmartRenderer>
                    COMMENT "Copying ANGLE DLLs to output directory"
                )
                
                # Add preprocessor definition
                target_compile_definitions(SmartRenderer PUBLIC USE_ANGLE=1)
            else()
                message(WARNING "ANGLE libraries not found in ${ANGLE_LIB_DIR}")
            endif()
         elseif(PLATFORM_MACOS)
             set(ANGLE_LIB_DIR "${ANGLE_ROOT}/libs/mac")
             
             # macOS: Use .dylib libraries
             set(ANGLE_EGL_LIB "${ANGLE_LIB_DIR}/libEGL.dylib")
             set(ANGLE_GLES_LIB "${ANGLE_LIB_DIR}/libGLESv2.dylib")
             
             if(EXISTS "${ANGLE_EGL_LIB}" AND EXISTS "${ANGLE_GLES_LIB}")
                 message(STATUS "Found ANGLE EGL: ${ANGLE_EGL_LIB}")
                 message(STATUS "Found ANGLE GLESv2: ${ANGLE_GLES_LIB}")
                 target_link_libraries(SmartRenderer PRIVATE ${ANGLE_EGL_LIB} ${ANGLE_GLES_LIB})
                 
                 # Copy dylibs to output directory
                 add_custom_command(TARGET SmartRenderer POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                         "${ANGLE_EGL_LIB}"
                         "${ANGLE_GLES_LIB}"
                         $<TARGET_FILE_DIR:SmartRenderer>
                     COMMENT "Copying ANGLE dylibs to output directory"
                 )
                 
                 target_compile_definitions(SmartRenderer PUBLIC USE_ANGLE=1)
             else()
                 message(WARNING "ANGLE libraries not found in ${ANGLE_LIB_DIR}")
             endif()
        endif()
    else()
        message(WARNING "ANGLE headers not found at ${ANGLE_INCLUDE_DIR}")
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(SmartRenderer PRIVATE /W4 /WX-)
else()
    target_compile_options(SmartRenderer PRIVATE -Wall -Wextra -Wpedantic)
endif()
