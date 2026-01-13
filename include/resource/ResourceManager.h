#pragma once

#include "TextureData.h"
#include <string>
#include <unordered_map>

namespace SmartRenderer {

// ResourceManager: Pure data loading and caching
// Completely decoupled from Renderer - no knowledge of GPU resources
// Users create GPU resources themselves using the cached data
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    // Load and cache resource data (no GPU resources created)
    bool LoadTextureData(const std::string& path, TextureData& outData);
    bool LoadShaderSource(const std::string& name,
                         const std::string& vertexPath,
                         const std::string& fragmentPath,
                         ShaderSource& outSource);
    bool LoadMeshData(const std::string& path); // TODO: implement

    // Check if resource is cached
    bool HasTextureData(const std::string& path) const;
    bool HasShaderSource(const std::string& name) const;

    // Get cached resource data
    const TextureData* GetTextureData(const std::string& path) const;
    const ShaderSource* GetShaderSource(const std::string& name) const;

    // Unload cached data
    void UnloadTextureData(const std::string& path);
    void UnloadShaderSource(const std::string& name);
    void UnloadMeshData(const std::string& path);

    // Clear all cached data
    void Clear();

private:
    // Cache resource data (not GPU resources)
    std::unordered_map<std::string, TextureData> m_textureDataCache;
    std::unordered_map<std::string, ShaderSource> m_shaderSourceCache;
    // TODO: mesh data cache
};

} // namespace SmartRenderer
