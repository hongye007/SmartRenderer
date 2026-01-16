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

    # ECS demo
    add_executable(ecs_demo
        ecs_demo/main.cpp
    )
    target_link_libraries(ecs_demo PRIVATE SmartRenderer)

    # Resource handle demo
    add_executable(resource_handle_demo
        resource_handle_demo/main.cpp
    )
    target_link_libraries(resource_handle_demo PRIVATE SmartRenderer)

    # Resource factory demo
    add_executable(resource_factory_demo
        resource_factory_demo/main.cpp
    )
    target_link_libraries(resource_factory_demo PRIVATE SmartRenderer)

    # Image outline sample
    add_executable(image_outline
        image_outline/main.cpp
    )
    target_link_libraries(image_outline PRIVATE SmartRenderer)

    if(PLATFORM_WINDOWS)
        target_link_libraries(window_test PRIVATE user32 gdi32)
        target_link_libraries(basic_triangle PRIVATE user32 gdi32)
        target_link_libraries(texture_sample PRIVATE user32 gdi32)
        target_link_libraries(simple_scene PRIVATE user32 gdi32)
        target_link_libraries(render_graph_demo PRIVATE user32 gdi32)
        target_link_libraries(ecs_demo PRIVATE user32 gdi32)
        target_link_libraries(resource_handle_demo PRIVATE user32 gdi32)
        target_link_libraries(resource_factory_demo PRIVATE user32 gdi32)
        target_link_libraries(image_outline PRIVATE user32 gdi32)
        
        # Copy ANGLE DLLs to sample output directories
        if(USE_ANGLE)
            set(ANGLE_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/gl-env/angle/libs/windows")
            foreach(SAMPLE_TARGET window_test basic_triangle texture_sample simple_scene render_graph_demo ecs_demo resource_handle_demo resource_factory_demo image_outline)
                add_custom_command(TARGET ${SAMPLE_TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${ANGLE_LIB_DIR}/libEGL.dll"
                        "${ANGLE_LIB_DIR}/libGLESv2.dll"
                        $<TARGET_FILE_DIR:${SAMPLE_TARGET}>
                    COMMENT "Copying ANGLE DLLs for ${SAMPLE_TARGET}"
                )
            endforeach()
        endif()
    endif()
endif()
