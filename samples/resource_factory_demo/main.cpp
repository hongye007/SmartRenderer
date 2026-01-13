#include "SmartRenderer.h"
#include "resource/ResourceFactory.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Buffer.h"
#include "rendering/VertexArray.h"
#include <iostream>
#include <memory>
#include <vector>
#include <cstddef>

using namespace SmartRenderer;

int main() {
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.appName = "Resource Factory Demo";

    auto platform = CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }
    
    auto window = platform->CreateWindow(1024, 768, "Resource Factory Demo - SmartRenderer");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    
    window->Show();

    SmartRenderer::RendererConfig renderConfig;
    auto renderer = SmartRenderer::CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }

    // Create ResourceFactory - unified resource management interface
    ResourceFactory resourceFactory(renderer.get());
    
    // Load texture using ResourceFactory (one-step operation)
    std::string imagePath = "/Users/admin/Downloads/test.jpg";
    TextureHandle texture = resourceFactory.LoadTexture(imagePath, "main_texture");
    
    if (!texture) {
        std::cerr << "Failed to load texture, creating fallback..." << std::endl;
        
        // Create fallback texture using ResourceFactory
        const int texWidth = 256;
        const int texHeight = 256;
        std::vector<uint8_t> testData(texWidth * texHeight * 4);
        for (int y = 0; y < texHeight; ++y) {
            for (int x = 0; x < texWidth; ++x) {
                int idx = (y * texWidth + x) * 4;
                testData[idx + 0] = static_cast<uint8_t>((x * 255) / texWidth);
                testData[idx + 1] = static_cast<uint8_t>((y * 255) / texHeight);
                testData[idx + 2] = 128;
                testData[idx + 3] = 255;
            }
        }
        
        texture = resourceFactory.CreateTexture("main_texture", texWidth, texHeight, 0, testData.data());
        if (!texture) {
            std::cerr << "Failed to create fallback texture" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Successfully loaded texture from: " << imagePath << std::endl;
    }

    // Create shader using ResourceFactory
    const char* vertexShaderSource = R"(
#version 100
precision mediump float;

attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

    const char* fragmentShaderSource = R"(
#version 100
precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D uTexture;

void main() {
    gl_FragColor = texture2D(uTexture, vTexCoord);
}
)";

    ShaderHandle shader = resourceFactory.CreateShader("texture_shader", 
                                                       vertexShaderSource, 
                                                       fragmentShaderSource);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to create shader" << std::endl;
        return 1;
    }

    // Create fullscreen quad
    struct Vertex {
        float position[2];
        float texCoord[2];
    };
    
    Vertex quadVertices[] = {
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}}
    };

    // Create and register buffer
    Buffer* vertexBuffer = renderer->CreateBuffer(sizeof(quadVertices), quadVertices);
    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return 1;
    }
    BufferHandle vertexBufferHandle = resourceFactory.RegisterBuffer("quad_vertices", vertexBuffer);

    // Create and register vertex array
    VertexArray* vertexArray = renderer->CreateVertexArray();
    if (!vertexArray) {
        std::cerr << "Failed to create vertex array" << std::endl;
        return 1;
    }
    VertexArrayHandle vertexArrayHandle = resourceFactory.RegisterVertexArray("quad_vao", vertexArray);

    // Setup vertex array
    vertexArrayHandle->SetVertexBuffer(vertexBufferHandle.Get(), 0, 2, sizeof(Vertex), offsetof(Vertex, position));
    vertexArrayHandle->SetVertexBuffer(vertexBufferHandle.Get(), 1, 2, sizeof(Vertex), offsetof(Vertex, texCoord));

    // Print statistics
    std::cout << "\nResource Factory Statistics:" << std::endl;
    std::cout << "  Textures: " << resourceFactory.GetTextureCount() << std::endl;
    std::cout << "  Shaders: " << resourceFactory.GetShaderCount() << std::endl;

    // Main render loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.2f, 0.2f, 0.3f, 1.0f));

        // Use handles to access resources
        renderer->BindShader(shader.Get());
        renderer->BindTexture(texture.Get(), 0);
        shader->SetUniform("uTexture", 0);

        renderer->BindVertexArray(vertexArrayHandle.Get());
        renderer->DrawArrays(0, 4);

        renderer->EndFrame();
        renderer->Present();
    }

    // Cleanup order is critical:
    // 1. Unbind all resources first
    renderer->BindShader(nullptr);
    renderer->BindTexture(nullptr, 0);
    renderer->BindVertexArray(nullptr);
    
    // 2. Clear ResourceFactory BEFORE shutting down renderer
    //    This ensures all resources are destroyed while renderer is still valid
    resourceFactory.Clear();
    
    // 3. Invalidate renderer in registry to prevent any remaining handles from accessing it
    resourceFactory.GetRegistry().InvalidateRenderer();
    
    // 4. Ensure all local resource handles are released
    shader.Reset();
    texture.Reset();
    vertexBufferHandle.Reset();
    vertexArrayHandle.Reset();
    
    // 5. Shutdown renderer (after all resources are destroyed)
    renderer->Shutdown();
    renderer.reset();
    
    // 6. Destroy window
    window->Destroy();
    
    // 7. Shutdown platform
    platform->Shutdown();
    platform.reset();
    
    std::cout << "\nAll resources automatically cleaned up by ResourceFactory!" << std::endl;
    
    return 0;
}
