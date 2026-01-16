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

// Vertex shader for fullscreen quad
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

// Fragment shader for outline effect
const char* fragmentShaderSource = R"(
#version 100
precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D uOriginalTexture;
uniform sampler2D uMaskTexture;
uniform vec4 uTextureSize; // vec4(x, y, 0, 0) - using vec4 for API compatibility
uniform float uOutlineWidth;
uniform vec4 uOutlineColor; // vec4(r, g, b, 0) - using vec4 for API compatibility

void main() {
    vec2 texSize = uTextureSize.xy;
    vec2 texelSize = 1.0 / texSize;
    vec3 outlineColor = uOutlineColor.rgb;
    
    // Sample original texture
    vec4 originalColor = texture2D(uOriginalTexture, vTexCoord);
    
    // Sample mask texture (use red channel as mask)
    float maskValue = texture2D(uMaskTexture, vTexCoord).r;
    
    // Check surrounding pixels for edge detection
    float outline = 0.0;
    float threshold = 0.5; // Mask threshold
    
    // Sample surrounding pixels
    vec2 offsets[8];
    offsets[0] = vec2(-uOutlineWidth, -uOutlineWidth) * texelSize;
    offsets[1] = vec2( 0.0, -uOutlineWidth) * texelSize;
    offsets[2] = vec2( uOutlineWidth, -uOutlineWidth) * texelSize;
    offsets[3] = vec2(-uOutlineWidth,  0.0) * texelSize;
    offsets[4] = vec2( uOutlineWidth,  0.0) * texelSize;
    offsets[5] = vec2(-uOutlineWidth,  uOutlineWidth) * texelSize;
    offsets[6] = vec2( 0.0,  uOutlineWidth) * texelSize;
    offsets[7] = vec2( uOutlineWidth,  uOutlineWidth) * texelSize;
    
    // Check if current pixel is inside mask
    bool insideMask = maskValue > threshold;
    
    // Check if any surrounding pixel is outside mask
    for (int i = 0; i < 8; i++) {
        float neighborMask = texture2D(uMaskTexture, vTexCoord + offsets[i]).r;
        bool neighborInside = neighborMask > threshold;
        
        // If current is inside but neighbor is outside, draw outline
        if (insideMask && !neighborInside) {
            outline = 1.0;
            break;
        }
    }
    
    // Mix original color with outline color
    vec3 finalColor = mix(originalColor.rgb, outlineColor, outline);
    
    gl_FragColor = vec4(finalColor, originalColor.a);
}
)";

int main() {
    std::cout << "=== Image Outline Sample ===" << std::endl;
    
    SmartRenderer::PlatformConfig config;
    config.windowWidth = 1024;
    config.windowHeight = 768;
    config.appName = "Image Outline Sample";

    auto platform = CreatePlatform();
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }
    
    // Create window
    auto window = platform->CreateWindow(1024, 768, "Image Outline - SmartRenderer");
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

    // Load textures using ResourceManager
    ResourceManager resourceManager;
    
    std::string originalImagePath = "/Users/admin/Downloads/shuye_ori.jpeg";
    std::string maskImagePath = "/Users/admin/Downloads/shuye.jpg";
    
    Texture* originalTexture = nullptr;
    Texture* maskTexture = nullptr;
    
    // Load original image
    TextureData originalData;
    if (resourceManager.LoadTextureData(originalImagePath, originalData)) {
        originalTexture = renderer->CreateTexture(
            originalData.width,
            originalData.height,
            originalData.format,
            originalData.data.data()
        );
        std::cout << "Loaded original image: " << originalImagePath << std::endl;
        std::cout << "  Size: " << originalData.width << "x" << originalData.height << std::endl;
    } else {
        std::cerr << "Failed to load original image: " << originalImagePath << std::endl;
        return 1;
    }
    
    // Load mask image
    TextureData maskData;
    if (resourceManager.LoadTextureData(maskImagePath, maskData)) {
        maskTexture = renderer->CreateTexture(
            maskData.width,
            maskData.height,
            maskData.format,
            maskData.data.data()
        );
        std::cout << "Loaded mask image: " << maskImagePath << std::endl;
        std::cout << "  Size: " << maskData.width << "x" << maskData.height << std::endl;
    } else {
        std::cerr << "Failed to load mask image: " << maskImagePath << std::endl;
        return 1;
    }
    
    if (!originalTexture || !maskTexture) {
        std::cerr << "Failed to create textures" << std::endl;
        return 1;
    }

    // Create shader
    Shader* shader = renderer->CreateShader(vertexShaderSource, fragmentShaderSource);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to create shader" << std::endl;
        return 1;
    }
    std::cout << "Shader created successfully" << std::endl;

    // Create fullscreen quad vertices (position + UV)
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

    // Create index buffer for two triangles
    // Triangle 1: bottom-left, bottom-right, top-left (0, 1, 2)
    // Triangle 2: bottom-right, top-right, top-left (1, 3, 2)
    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        1, 3, 2   // Second triangle
    };
    
    Buffer* indexBuffer = renderer->CreateBuffer(sizeof(indices), indices);
    if (!indexBuffer) {
        std::cerr << "Failed to create index buffer" << std::endl;
        return 1;
    }

    // Create vertex array
    VertexArray* vertexArray = renderer->CreateVertexArray();
    if (!vertexArray) {
        std::cerr << "Failed to create vertex array" << std::endl;
        return 1;
    }
    
    // Set up vertex attributes using offsetof for proper alignment
    // Position: location 0, 2 floats, stride sizeof(Vertex), offset 0
    // TexCoord: location 1, 2 floats, stride sizeof(Vertex), offset 8
    vertexArray->SetVertexBuffer(vertexBuffer, 0, 2, sizeof(Vertex), offsetof(Vertex, position));
    vertexArray->SetVertexBuffer(vertexBuffer, 1, 2, sizeof(Vertex), offsetof(Vertex, texCoord));
    
    // Set index buffer
    vertexArray->SetIndexBuffer(indexBuffer);

    // Set viewport to match window size
    int windowWidth, windowHeight;
    window->GetSize(windowWidth, windowHeight);
    renderer->SetViewport(0, 0, windowWidth, windowHeight);

    std::cout << "Starting main loop..." << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;

    // Main loop
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));

        // Bind shader
        renderer->BindShader(shader);
        
        // Set uniforms
        // Texture size for proper sampling (vec2: x, y, 0, 0)
        shader->SetUniform("uTextureSize", 
                          static_cast<float>(originalData.width), 
                          static_cast<float>(originalData.height),
                          0.0f, 0.0f);
        // Outline width (in pixels, converted to texture coordinates)
        shader->SetUniform("uOutlineWidth", 2.0f);
        // Outline color (red) - vec3 as vec4 (x, y, z, 0)
        shader->SetUniform("uOutlineColor", 1.0f, 0.0f, 0.0f, 0.0f);
        
        // Bind textures
        renderer->BindTexture(originalTexture, 0);
        renderer->BindTexture(maskTexture, 1);
        
        // Set texture uniforms
        shader->SetUniform("uOriginalTexture", 0);
        shader->SetUniform("uMaskTexture", 1);
        
        // Bind vertex array and draw using indices
        renderer->BindVertexArray(vertexArray);
        renderer->DrawElements(6, 0); // Draw 6 indices (2 triangles)

        renderer->EndFrame();
        renderer->Present();
    }

    std::cout << "Shutting down..." << std::endl;
    
    // Cleanup
    renderer->DestroyShader(shader);
    renderer->DestroyTexture(originalTexture);
    renderer->DestroyTexture(maskTexture);
    renderer->DestroyBuffer(vertexBuffer);
    renderer->DestroyBuffer(indexBuffer);
    renderer->DestroyVertexArray(vertexArray);
    renderer->Shutdown();
    platform->Shutdown();
    
    std::cout << "Image outline sample completed successfully" << std::endl;
    return 0;
}
