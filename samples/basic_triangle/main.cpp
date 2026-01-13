#include "SmartRenderer.h"
#include "rendering/Shader.h"
#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"
#include <iostream>

using namespace SmartRenderer;

// Simple triangle shader
const char* vertexShaderSource = R"(
#version 100
precision mediump float;

attribute vec2 aPosition;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 100
precision mediump float;

void main() {
    gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); // Orange color
}
)";

int main() {
    // Initialize platform
    PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Basic Triangle Sample";

    auto platform = CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    // Create window
    auto window = platform->CreateWindow(800, 600, "Basic Triangle");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    window->Show();

    // Initialize renderer
    RendererConfig renderConfig;
    auto renderer = CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }

    // Create shader
    Shader* shader = renderer->CreateShader(vertexShaderSource, fragmentShaderSource);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to create shader" << std::endl;
        return 1;
    }

    // Create triangle vertices (position only)
    float vertices[] = {
        -0.5f, -0.5f,  // Bottom left
         0.5f, -0.5f,  // Bottom right
         0.0f,  0.5f   // Top
    };

    // Create vertex buffer
    Buffer* vertexBuffer = renderer->CreateBuffer(sizeof(vertices), vertices);
    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return 1;
    }

    // Create vertex array
    VertexArray* vertexArray = renderer->CreateVertexArray();
    if (!vertexArray) {
        std::cerr << "Failed to create vertex array" << std::endl;
        return 1;
    }

    // Set up vertex attributes (position: 2 floats, stride: 2 * sizeof(float), offset: 0)
    vertexArray->SetVertexBuffer(vertexBuffer, 0, 2, 2 * sizeof(float), 0);

    // Main loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.2f, 0.3f, 0.4f, 1.0f));
        
        // Render triangle
        renderer->BindShader(shader);
        renderer->BindVertexArray(vertexArray);
        renderer->DrawArrays(0, 3); // Draw 3 vertices starting from index 0
        
        renderer->EndFrame();
        renderer->Present();
    }

    // Cleanup
    renderer->DestroyShader(shader);
    renderer->DestroyBuffer(vertexBuffer);
    renderer->DestroyVertexArray(vertexArray);
    renderer->Shutdown();
    platform->Shutdown();
    return 0;
}
