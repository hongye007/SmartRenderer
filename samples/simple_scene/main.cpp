#include "SmartRenderer.h"
#include "rendering/Shader.h"
#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"
#include <iostream>

using namespace SmartRenderer;

// Simple colored triangle shader
const char* vertexShaderSource = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = vec4(aPosition, 0.0, 1.0);
})";

const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;

in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
})";

int main() {
    std::cout << "=== Simple Scene Sample ===" << std::endl;
    
    // Initialize platform
    PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Simple Scene Sample";

    auto platform = CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    // Create window
    auto window = platform->CreateWindow(800, 600, "Simple Scene - Colored Triangle");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    // Show window
    window->Show();
    std::cout << "Window created and shown" << std::endl;

    // Initialize renderer
    std::cout << "Creating renderer..." << std::endl;
    RendererConfig renderConfig;
    auto renderer = CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        std::cerr << "This may be due to ANGLE initialization failure" << std::endl;
        std::cerr << "Check that libEGL.dll and libGLESv2.dll are in the same directory" << std::endl;
        return 1;
    }

    std::cout << "Renderer initialized successfully" << std::endl;

    // Create shader
    auto shader = renderer->CreateShader(vertexShaderSource, fragmentShaderSource);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to create shader" << std::endl;
        return 1;
    }
    std::cout << "Shader created successfully" << std::endl;

    // Triangle vertices (position + color)
    // Each vertex: 2 floats for position (x, y) + 3 floats for color (r, g, b) = 5 floats
    float vertices[] = {
        // Position (x, y)    // Color (RGB)
        -0.5f, -0.5f,          1.0f, 0.0f, 0.0f,  // Bottom left (red)
         0.5f, -0.5f,          0.0f, 1.0f, 0.0f,  // Bottom right (green)
         0.0f,  0.5f,          0.0f, 0.0f, 1.0f   // Top (blue)
    };

    // Create vertex buffer
    auto vertexBuffer = renderer->CreateBuffer(sizeof(vertices), vertices);
    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return 1;
    }
    std::cout << "Vertex buffer created successfully" << std::endl;

    // Create vertex array
    auto vertexArray = renderer->CreateVertexArray();
    if (!vertexArray) {
        std::cerr << "Failed to create vertex array" << std::endl;
        return 1;
    }
    
    // Set up vertex attributes
    // Position: location 0, 2 floats, stride 5*sizeof(float), offset 0
    // Color: location 1, 3 floats, stride 5*sizeof(float), offset 2*sizeof(float)
    vertexArray->SetVertexBuffer(vertexBuffer, 0, 2, 5 * sizeof(float), 0);
    vertexArray->SetVertexBuffer(vertexBuffer, 1, 3, 5 * sizeof(float), 2 * sizeof(float));
    std::cout << "Vertex array configured successfully" << std::endl;

    std::cout << "Starting main loop..." << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;

    // Main loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));

        // Draw triangle
        renderer->BindShader(shader);
        renderer->BindVertexArray(vertexArray);
        renderer->DrawArrays(0, 3); // Draw 3 vertices

        renderer->EndFrame();
        renderer->Present();
    }

    std::cout << "Shutting down..." << std::endl;
    renderer->Shutdown();
    platform->Shutdown();
    
    std::cout << "Simple scene sample completed successfully" << std::endl;
    return 0;
}
