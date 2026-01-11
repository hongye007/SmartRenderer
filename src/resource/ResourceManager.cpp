#include "resource/ResourceManager.h"
#include "resource/Mesh.h"
#include "resource/AssetLoader.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "core/Renderer.h"
#include <stdexcept>
#include <iostream>

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

    // Load texture data (simplified - would need image decoder)
    // For now, create a placeholder texture
    std::vector<uint8_t> data = AssetLoader::LoadFile(path);
    if (data.empty()) {
        std::cerr << "ResourceManager: Failed to load texture: " << path << std::endl;
        return nullptr;
    }

    // In real implementation, would decode image format (PNG, JPEG, etc.)
    // For now, create a 1x1 placeholder texture
    uint8_t placeholderData[4] = {255, 255, 255, 255};
    Texture* texture = m_renderer->CreateTexture(1, 1, 0, placeholderData);
    if (texture) {
        m_textures[path] = std::unique_ptr<Texture>(texture);
        return texture;
    }

    return nullptr;
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
