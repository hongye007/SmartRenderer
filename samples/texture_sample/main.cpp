#include "SmartRenderer.h"
#include "resource/ResourceManager.h"
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
    config.appName = "Texture Sample";

    auto platform = CreatePlatform(PlatformType::macOS);
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }
    
    // Create window
    auto window = platform->CreateWindow(1024, 768, "Texture Sample - SmartRenderer");
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

    // Load texture using ResourceManager (completely decoupled from Renderer)
    ResourceManager resourceManager;
    
    // Try to load a real image file, fallback to generated texture if not found
    Texture* texture = nullptr;
    bool textureFromResourceManager = false;
    std::string imagePath = "/Users/admin/Downloads/test.jpg";
    
    // Load texture data (ResourceManager only handles data loading)
    TextureData textureData;
    if (resourceManager.LoadTextureData(imagePath, textureData)) {
        // Create GPU texture from cached data (user creates it via Renderer)
        texture = renderer->CreateTexture(
            textureData.width,
            textureData.height,
            textureData.format,
            textureData.data.data()
        );
        textureFromResourceManager = true;
    }
    
    if (!texture) {
        std::cerr << "Failed to load texture from: " << imagePath << std::endl;
        std::cerr << "Creating a test gradient texture instead..." << std::endl;
        
        // Create a test texture (256x256 RGBA gradient) as fallback
        const int texWidth = 256;
        const int texHeight = 256;
        std::vector<uint8_t> textureData(texWidth * texHeight * 4);
        for (int y = 0; y < texHeight; ++y) {
            for (int x = 0; x < texWidth; ++x) {
                int idx = (y * texWidth + x) * 4;
                textureData[idx + 0] = static_cast<uint8_t>((x * 255) / texWidth);  // R
                textureData[idx + 1] = static_cast<uint8_t>((y * 255) / texHeight); // G
                textureData[idx + 2] = 128;                                          // B
                textureData[idx + 3] = 255;                                          // A
            }
        }
        
        texture = renderer->CreateTexture(texWidth, texHeight, 0, textureData.data());
        textureFromResourceManager = false; // Created directly, not from ResourceManager
        if (!texture) {
            std::cerr << "Failed to create fallback texture" << std::endl;
            return 1;
        }
    } else {
        textureFromResourceManager = true; // Loaded from ResourceManager
        std::cout << "Successfully loaded texture from: " << imagePath << std::endl;
    }

    // Create shader for texture rendering (GLSL ES 100)
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

    Shader* shader = renderer->CreateShader(vertexShaderSource, fragmentShaderSource);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to create shader" << std::endl;
        return 1;
    }

    // Create fullscreen quad vertices (position + UV)
    // Positions: -1 to 1 in NDC, TexCoords: 0 to 1
    struct Vertex {
        float position[2];
        float texCoord[2];
    };
    
    Vertex quadVertices[] = {
        // Bottom-left
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        // Bottom-right
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        // Top-left
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        // Top-right
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}}
    };

    // Create vertex buffer
    Buffer* vertexBuffer = renderer->CreateBuffer(sizeof(quadVertices), quadVertices);
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

    // Set up vertex attributes
    // Position attribute (location 0): 2 floats, stride 16 bytes, offset 0
    vertexArray->SetVertexBuffer(vertexBuffer, 0, 2, sizeof(Vertex), offsetof(Vertex, position));
    // TexCoord attribute (location 1): 2 floats, stride 16 bytes, offset 8
    vertexArray->SetVertexBuffer(vertexBuffer, 1, 2, sizeof(Vertex), offsetof(Vertex, texCoord));

    // Main render loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.2f, 0.2f, 0.3f, 1.0f));

        // Bind shader
        renderer->BindShader(shader);
        
        // Bind texture
        renderer->BindTexture(texture, 0);
        shader->SetUniform("uTexture", 0);

        // Bind vertex array and draw
        renderer->BindVertexArray(vertexArray);
        renderer->DrawArrays(0, 4);

        renderer->EndFrame();
        renderer->Present();
    }

    // Cleanup resources in correct order
    // 1. Unbind all resources first to avoid using destroyed resources
    renderer->BindShader(nullptr);
    renderer->BindTexture(nullptr, 0);
    renderer->BindVertexArray(nullptr);
    
    // 2. Destroy rendering resources that were created directly
    renderer->DestroyVertexArray(vertexArray);
    renderer->DestroyBuffer(vertexBuffer);
    renderer->DestroyShader(shader);
    
    // 3. Destroy texture (ResourceManager only caches data, doesn't manage GPU resources)
    // So we need to destroy the texture regardless of how it was created
    if (texture != nullptr) {
        renderer->DestroyTexture(texture);
    }
    
    // 4. Clear ResourceManager cache (only clears data cache, not GPU resources)
    // ResourceManager is completely decoupled - no Renderer dependency
    resourceManager.Clear();
    
    // 5. Shutdown renderer (this will clean up OpenGL context)
    // Must be done after all resources are destroyed
    renderer->Shutdown();
    renderer.reset();
    
    // 6. Destroy window (after renderer is shut down)
    window->Destroy();
    
    // 7. Shutdown platform (last)
    platform->Shutdown();
    platform.reset();
    
    return 0;
}
