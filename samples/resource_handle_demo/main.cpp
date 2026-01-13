#include "SmartRenderer.h"
#include "resource/ResourceManager.h"
#include "resource/ResourceRegistry.h"
#include "resource/ResourceHelper.h"
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
    config.appName = "Resource Handle Demo";

    auto platform = CreatePlatform(PlatformType::macOS);
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }
    
    // Create window
    auto window = platform->CreateWindow(1024, 768, "Resource Handle Demo - SmartRenderer");
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

    // Create ResourceRegistry to manage all GPU resources
    // This will automatically clean up all resources when it goes out of scope
    ResourceRegistry resourceRegistry(renderer.get());
    
    // ResourceManager for loading data (completely decoupled)
    ResourceManager resourceManager;
    
    // Load texture data
    Texture* texture = nullptr;
    std::string imagePath = "/Users/admin/Downloads/test.jpg";
    
    TextureData textureData;
    if (resourceManager.LoadTextureData(imagePath, textureData)) {
        // Create GPU texture from cached data
        texture = renderer->CreateTexture(
            textureData.width,
            textureData.height,
            textureData.format,
            textureData.data.data()
        );
        std::cout << "Successfully loaded texture from: " << imagePath << std::endl;
    }
    
    // Fallback to generated texture if loading failed
    if (!texture) {
        std::cerr << "Failed to load texture from: " << imagePath << std::endl;
        std::cerr << "Creating a test gradient texture instead..." << std::endl;
        
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
        
        texture = renderer->CreateTexture(texWidth, texHeight, 0, testData.data());
        if (!texture) {
            std::cerr << "Failed to create fallback texture" << std::endl;
            return 1;
        }
    }

    // Register texture to registry (returns handle with automatic cleanup)
    TextureHandle textureHandle = resourceRegistry.RegisterTexture("main_texture", texture);
    if (!textureHandle) {
        std::cerr << "Failed to register texture" << std::endl;
        return 1;
    }

    // Create shader
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

    // Register shader to registry
    ShaderHandle shaderHandle = resourceRegistry.RegisterShader("texture_shader", shader);
    if (!shaderHandle) {
        std::cerr << "Failed to register shader" << std::endl;
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

    // Create and register vertex buffer
    Buffer* vertexBuffer = renderer->CreateBuffer(sizeof(quadVertices), quadVertices);
    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return 1;
    }
    BufferHandle vertexBufferHandle = resourceRegistry.RegisterBuffer("quad_vertices", vertexBuffer);

    // Create and register vertex array
    VertexArray* vertexArray = renderer->CreateVertexArray();
    if (!vertexArray) {
        std::cerr << "Failed to create vertex array" << std::endl;
        return 1;
    }
    VertexArrayHandle vertexArrayHandle = resourceRegistry.RegisterVertexArray("quad_vao", vertexArray);

    // Setup vertex array
    // Position attribute (location 0): 2 floats, stride 16 bytes, offset 0
    vertexArrayHandle->SetVertexBuffer(vertexBufferHandle.Get(), 0, 2, sizeof(Vertex), offsetof(Vertex, position));
    // TexCoord attribute (location 1): 2 floats, stride 16 bytes, offset 8
    vertexArrayHandle->SetVertexBuffer(vertexBufferHandle.Get(), 1, 2, sizeof(Vertex), offsetof(Vertex, texCoord));

    // Print registry statistics
    std::cout << "\nResource Registry Statistics:" << std::endl;
    std::cout << "  Textures: " << resourceRegistry.GetTextureCount() << std::endl;
    std::cout << "  Shaders: " << resourceRegistry.GetShaderCount() << std::endl;
    std::cout << "  Buffers: " << resourceRegistry.GetBufferCount() << std::endl;
    std::cout << "  Vertex Arrays: " << resourceRegistry.GetVertexArrayCount() << std::endl;

    // Main render loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.2f, 0.2f, 0.3f, 1.0f));

        // Use handles to access resources (automatic lifetime management)
        renderer->BindShader(shaderHandle.Get());
        renderer->BindTexture(textureHandle.Get(), 0);
        shaderHandle->SetUniform("uTexture", 0);

        renderer->BindVertexArray(vertexArrayHandle.Get());
        renderer->DrawArrays(0, 4);

        renderer->EndFrame();
        renderer->Present();
    }

    // Demonstrate getting resources by name
    {
        TextureHandle retrievedTexture = resourceRegistry.GetTexture("main_texture");
        if (retrievedTexture) {
            std::cout << "\nSuccessfully retrieved texture by name" << std::endl;
        }
        // retrievedTexture goes out of scope here, but resource is still in registry
    }

    // Cleanup order is critical:
    // 1. Unbind all resources first
    renderer->BindShader(nullptr);
    renderer->BindTexture(nullptr, 0);
    renderer->BindVertexArray(nullptr);
    
    // 2. Clear ResourceManager cache (only data, not GPU resources)
    resourceManager.Clear();
    
    // 3. Clear ResourceRegistry BEFORE shutting down renderer
    //    This ensures all resources are destroyed while renderer is still valid
    //    All resource handles must be released before renderer shutdown
    resourceRegistry.Clear();
    
    // 4. Ensure all local resource handles are released before renderer shutdown
    //    (They should already be cleared by resourceRegistry.Clear(), but we reset them explicitly)
    shaderHandle.Reset();
    textureHandle.Reset();
    vertexBufferHandle.Reset();
    vertexArrayHandle.Reset();
    
    // 5. Invalidate renderer in registry to prevent any remaining handles from accessing it
    resourceRegistry.InvalidateRenderer();
    
    // 6. Shutdown renderer (after all resources are destroyed)
    renderer->Shutdown();
    renderer.reset();
    
    // 5. Destroy window
    window->Destroy();
    
    // 6. Shutdown platform
    platform->Shutdown();
    platform.reset();
    
    std::cout << "\nAll resources automatically cleaned up by ResourceRegistry!" << std::endl;
    
    return 0;
}
