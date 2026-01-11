if(BUILD_SAMPLES)
    # Window test sample (no rendering, just platform/window testing)
    add_executable(window_test
        window_test/main.cpp
    )
    target_link_libraries(window_test PRIVATE SmartRenderer)

    # Basic triangle sample
    add_executable(basic_triangle
        basic_triangle/main.cpp
    )
    target_link_libraries(basic_triangle PRIVATE SmartRenderer)

    # Texture sample
    add_executable(texture_sample
        texture_sample/main.cpp
    )
    target_link_libraries(texture_sample PRIVATE SmartRenderer)

    # Simple scene sample
    add_executable(simple_scene
        simple_scene/main.cpp
    )
    target_link_libraries(simple_scene PRIVATE SmartRenderer)

    # Render graph demo
    add_executable(render_graph_demo
        render_graph_demo/main.cpp
    )
    target_link_libraries(render_graph_demo PRIVATE SmartRenderer)

    if(PLATFORM_WINDOWS)
        target_link_libraries(window_test PRIVATE user32 gdi32)
        target_link_libraries(basic_triangle PRIVATE user32 gdi32)
        target_link_libraries(texture_sample PRIVATE user32 gdi32)
        target_link_libraries(simple_scene PRIVATE user32 gdi32)
        target_link_libraries(render_graph_demo PRIVATE user32 gdi32)
    endif()
endif()
