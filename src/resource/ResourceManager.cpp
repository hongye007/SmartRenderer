#include "resource/ResourceManager.h"
#include "resource/AssetLoader.h"
#include <iostream>
#include <cstring>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace SmartRenderer {

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
    Clear();
}

// Load and cache shader source data
bool ResourceManager::LoadShaderSource(const std::string& name,
                                       const std::string& vertexPath,
                                       const std::string& fragmentPath,
                                       ShaderSource& outSource) {
    // Check if already cached
    auto it = m_shaderSourceCache.find(name);
    if (it != m_shaderSourceCache.end()) {
        outSource = it->second;
        return true;
    }

    // Load shader source files
    std::string vertexSource = AssetLoader::LoadTextFile(vertexPath);
    std::string fragmentSource = AssetLoader::LoadTextFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ResourceManager: Failed to load shader files" << std::endl;
        return false;
    }

    // Cache the source
    ShaderSource source;
    source.vertexSource = std::move(vertexSource);
    source.fragmentSource = std::move(fragmentSource);
    m_shaderSourceCache[name] = source;
    outSource = source;
    return true;
}

// Get cached shader source
const ShaderSource* ResourceManager::GetShaderSource(const std::string& name) const {
    auto it = m_shaderSourceCache.find(name);
    if (it != m_shaderSourceCache.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ResourceManager::HasShaderSource(const std::string& name) const {
    return m_shaderSourceCache.find(name) != m_shaderSourceCache.end();
}

// Load and cache texture data
bool ResourceManager::LoadTextureData(const std::string& path, TextureData& outData) {
    // Check if already cached
    auto it = m_textureDataCache.find(path);
    if (it != m_textureDataCache.end()) {
        outData = it->second;
        return true;
    }

    // Load image using stb_image
    int width, height, channels;
    unsigned char* imageData = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (!imageData) {
        std::cerr << "ResourceManager: Failed to load texture: " << path << std::endl;
        std::cerr << "  Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "ResourceManager: Invalid texture dimensions: " << width << "x" << height << std::endl;
        stbi_image_free(imageData);
        return false;
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
        return false;
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

    // Store in cache
    TextureData cachedData;
    cachedData.width = width;
    cachedData.height = height;
    cachedData.format = format;
    cachedData.data.assign(flippedData, flippedData + width * height * bytesPerPixel);
    
    // Free flipped image data
    delete[] flippedData;

    m_textureDataCache[path] = cachedData;
    outData = cachedData;
    return true;
}

// Get cached texture data
const TextureData* ResourceManager::GetTextureData(const std::string& path) const {
    auto it = m_textureDataCache.find(path);
    if (it != m_textureDataCache.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ResourceManager::HasTextureData(const std::string& path) const {
    return m_textureDataCache.find(path) != m_textureDataCache.end();
}

bool ResourceManager::LoadMeshData(const std::string& path) {
    // TODO: implement mesh data loading and caching
    (void)path; // Suppress unused parameter warning
    return false;
}

void ResourceManager::UnloadShaderSource(const std::string& name) {
    m_shaderSourceCache.erase(name);
}

void ResourceManager::UnloadTextureData(const std::string& path) {
    m_textureDataCache.erase(path);
}

void ResourceManager::UnloadMeshData(const std::string& path) {
    // TODO: implement mesh data cache
    (void)path; // Suppress unused parameter warning
}

void ResourceManager::Clear() {
    m_textureDataCache.clear();
    m_shaderSourceCache.clear();
    // TODO: clear mesh data cache
}

} // namespace SmartRenderer
