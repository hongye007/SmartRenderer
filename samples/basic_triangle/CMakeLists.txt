add_executable(basic_triangle
    main.cpp
)

target_link_libraries(basic_triangle PRIVATE SmartRenderer)

if(PLATFORM_WINDOWS)
    target_link_libraries(basic_triangle PRIVATE user32 gdi32)
endif()
