# Find OpenGL/OpenGL ES
# This is a simplified version - in production, use system FindOpenGL or ANGLE

if(USE_ANGLE)
    # ANGLE provides OpenGL ES
    message(STATUS "Using ANGLE for OpenGL ES")
    set(OPENGL_FOUND TRUE)
    set(OPENGL_ES_FOUND TRUE)
else()
    # Try to find system OpenGL
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        message(STATUS "Found system OpenGL")
    else()
        message(WARNING "OpenGL not found. Consider using ANGLE.")
    endif()
endif()
