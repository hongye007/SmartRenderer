add_executable(ecs_demo main.cpp)

target_link_libraries(ecs_demo PRIVATE SmartRenderer)

set_target_properties(ecs_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
