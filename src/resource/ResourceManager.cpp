#include "resource/ResourceManager.h"
#include "resource/Mesh.h"
#include "resource/AssetLoader.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "core/Renderer.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace SmartRenderer {

ResourceManager::ResourceManager()
    : m_renderer(nullptr) {
}

ResourceManager::~ResourceManager() {
    Clear();
}

Shader* ResourceManager::LoadShader(const std::string& name,
                                    const std::string& vertexPath,
                                    const std::string& fragmentPath) {
    // Check if already loaded
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.get();
    }

    if (!m_renderer) {
        std::cerr << "ResourceManager: Renderer not set" << std::endl;
        return nullptr;
    }

    // Load shader source files
    std::string vertexSource = AssetLoader::LoadTextFile(vertexPath);
    std::string fragmentSource = AssetLoader::LoadTextFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ResourceManager: Failed to load shader files" << std::endl;
        return nullptr;
    }

    // Create shader using renderer
    Shader* shader = m_renderer->CreateShader(vertexSource, fragmentSource);
    if (shader && shader->IsValid()) {
        m_shaders[name] = std::unique_ptr<Shader>(shader);
        return shader;
    }

    std::cerr << "ResourceManager: Failed to compile shader: " << name << std::endl;
    if (shader) {
        m_renderer->DestroyShader(shader);
    }
    return nullptr;
}

Texture* ResourceManager::LoadTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second.get();
    }

    if (!m_renderer) {
        std::cerr << "ResourceManager: Renderer not set" << std::endl;
        return nullptr;
    }

    // Load image using stb_image
    int width, height, channels;
    unsigned char* imageData = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (!imageData) {
        std::cerr << "ResourceManager: Failed to load texture: " << path << std::endl;
        std::cerr << "  Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "ResourceManager: Invalid texture dimensions: " << width << "x" << height << std::endl;
        stbi_image_free(imageData);
        return nullptr;
    }

    // Determine format: 0 = RGBA, 1 = RGB
    int format = 0; // Default to RGBA
    unsigned char* textureData = imageData;
    bool isStbAllocated = true; // Track if data is from stbi_load (needs stbi_image_free)
    int bytesPerPixel = channels;
    
    // Convert to RGBA if needed
    if (channels == 3) {
        // RGB format - use as is
        format = 1; // Use RGB format (0 = RGBA, 1 = RGB)
        bytesPerPixel = 3;
    } else if (channels == 4) {
        // Already RGBA
        format = 0;
        bytesPerPixel = 4;
    } else if (channels == 1) {
        // Grayscale -> RGBA conversion
        size_t rgbaSize = width * height * 4;
        textureData = new unsigned char[rgbaSize];
        isStbAllocated = false; // Now using new[] instead of stbi data
        for (int i = 0; i < width * height; ++i) {
            unsigned char gray = imageData[i];
            textureData[i * 4 + 0] = gray;
            textureData[i * 4 + 1] = gray;
            textureData[i * 4 + 2] = gray;
            textureData[i * 4 + 3] = 255;
        }
        stbi_image_free(imageData); // Free original grayscale data from stbi
        format = 0;
        bytesPerPixel = 4;
    } else {
        std::cerr << "ResourceManager: Unsupported channel count: " << channels << std::endl;
        stbi_image_free(imageData);
        return nullptr;
    }

    // Flip image vertically (OpenGL expects origin at bottom-left, images have origin at top-left)
    int rowSize = width * bytesPerPixel;
    unsigned char* flippedData = new unsigned char[width * height * bytesPerPixel];
    
    for (int y = 0; y < height; ++y) {
        int srcRow = y;
        int dstRow = height - 1 - y; // Flip vertically
        std::memcpy(flippedData + dstRow * rowSize, textureData + srcRow * rowSize, rowSize);
    }
    
    // Free original data
    if (isStbAllocated) {
        stbi_image_free(textureData);
    } else {
        delete[] textureData;
    }
    
    textureData = flippedData;

    // Create texture (this should copy the data, so we can free it after)
    // format: 0 = RGBA, 1 = RGB
    Texture* texture = m_renderer->CreateTexture(width, height, format, textureData);
    
    // Free flipped image data after texture creation
    delete[] textureData;

    if (!texture) {
        std::cerr << "ResourceManager: Failed to create texture from image data" << std::endl;
        return nullptr;
    }

    m_textures[path] = std::unique_ptr<Texture>(texture);
    return texture;
}

Mesh* ResourceManager::LoadMesh(const std::string& path) {
    auto it = m_meshes.find(path);
    if (it != m_meshes.end()) {
        return it->second.get();
    }
    // Stub implementation
    return nullptr;
}

void ResourceManager::UnloadShader(const std::string& name) {
    m_shaders.erase(name);
}

void ResourceManager::UnloadTexture(const std::string& path) {
    m_textures.erase(path);
}

void ResourceManager::UnloadMesh(const std::string& path) {
    m_meshes.erase(path);
}

void ResourceManager::Clear() {
    m_shaders.clear();
    m_textures.clear();
    m_meshes.clear();
}

} // namespace SmartRenderer
