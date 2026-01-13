#pragma once

#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "ResourceHandle.h"
#include "core/Renderer.h"
#include <string>
#include <memory>

namespace SmartRenderer {

// ResourceFactory: Unified resource management interface
// Combines ResourceManager (data loading) and ResourceRegistry (GPU resource management)
// Provides a single, safe, and convenient API for resource loading
class ResourceFactory {
public:
    // Initialize with renderer and optional data manager
    explicit ResourceFactory(Renderer* renderer, ResourceManager* dataManager = nullptr);
    ~ResourceFactory();

    // Load and register texture (one-step operation)
    // Returns handle that automatically manages lifetime
    TextureHandle LoadTexture(const std::string& path, const std::string& name = "");

    // Load and register shader (one-step operation)
    // Returns handle that automatically manages lifetime
    ShaderHandle LoadShader(const std::string& name,
                           const std::string& vertexPath,
                           const std::string& fragmentPath);

    // Create and register texture from data
    TextureHandle CreateTexture(const std::string& name,
                                int width, int height, int format,
                                const void* data = nullptr);

    // Create and register shader from source
    ShaderHandle CreateShader(const std::string& name,
                              const std::string& vertexSource,
                              const std::string& fragmentSource);

    // Get registered resources
    TextureHandle GetTexture(const std::string& name) const;
    ShaderHandle GetShader(const std::string& name) const;
    BufferHandle GetBuffer(const std::string& name) const;
    VertexArrayHandle GetVertexArray(const std::string& name) const;
    FramebufferHandle GetFramebuffer(const std::string& name) const;

    // Check if resource exists
    bool HasTexture(const std::string& name) const;
    bool HasShader(const std::string& name) const;

    // Register existing resources (for resources created outside factory)
    TextureHandle RegisterTexture(const std::string& name, Texture* texture);
    ShaderHandle RegisterShader(const std::string& name, Shader* shader);
    BufferHandle RegisterBuffer(const std::string& name, Buffer* buffer);
    VertexArrayHandle RegisterVertexArray(const std::string& name, VertexArray* vao);
    FramebufferHandle RegisterFramebuffer(const std::string& name, Framebuffer* framebuffer);

    // Clear all resources
    void Clear();

    // Get underlying registry (for advanced usage)
    ResourceRegistry& GetRegistry() { return m_registry; }
    const ResourceRegistry& GetRegistry() const { return m_registry; }

    // Get data manager (for advanced usage)
    ResourceManager& GetDataManager() { return m_dataManager; }
    const ResourceManager& GetDataManager() const { return m_dataManager; }

    // Statistics
    size_t GetTextureCount() const { return m_registry.GetTextureCount(); }
    size_t GetShaderCount() const { return m_registry.GetShaderCount(); }

private:
    ResourceRegistry m_registry;
    ResourceManager m_dataManager;
    ResourceManager* m_externalDataManager; // Optional external data manager
    Renderer* m_renderer;

    // Generate default name from path
    std::string GenerateNameFromPath(const std::string& path) const;
};

} // namespace SmartRenderer
