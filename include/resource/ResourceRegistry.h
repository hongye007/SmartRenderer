#pragma once

#include "ResourceHandle.h"
#include "core/Renderer.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

namespace SmartRenderer {

// Forward declarations
class Shader;
class Texture;
class Buffer;
class VertexArray;
class Framebuffer;

// Resource registry - centralized resource management with reference counting
// Tracks all GPU resources and automatically cleans them up
class ResourceRegistry {
public:
    explicit ResourceRegistry(Renderer* renderer);
    ~ResourceRegistry();

    // Register resources (returns handle)
    ShaderHandle RegisterShader(const std::string& name, Shader* shader);
    TextureHandle RegisterTexture(const std::string& name, Texture* texture);
    BufferHandle RegisterBuffer(const std::string& name, Buffer* buffer);
    VertexArrayHandle RegisterVertexArray(const std::string& name, VertexArray* vao);
    FramebufferHandle RegisterFramebuffer(const std::string& name, Framebuffer* framebuffer);

    // Get registered resources
    ShaderHandle GetShader(const std::string& name) const;
    TextureHandle GetTexture(const std::string& name) const;
    BufferHandle GetBuffer(const std::string& name) const;
    VertexArrayHandle GetVertexArray(const std::string& name) const;
    FramebufferHandle GetFramebuffer(const std::string& name) const;

    // Check if resource exists
    bool HasShader(const std::string& name) const;
    bool HasTexture(const std::string& name) const;
    bool HasBuffer(const std::string& name) const;
    bool HasVertexArray(const std::string& name) const;
    bool HasFramebuffer(const std::string& name) const;

    // Unregister resource (will be destroyed when last handle is released)
    void UnregisterShader(const std::string& name);
    void UnregisterTexture(const std::string& name);
    void UnregisterBuffer(const std::string& name);
    void UnregisterVertexArray(const std::string& name);
    void UnregisterFramebuffer(const std::string& name);

    // Clear all registered resources
    void Clear();

    // Invalidate renderer (call before renderer is destroyed)
    // This prevents resource handles from trying to destroy resources after renderer is gone
    void InvalidateRenderer();

    // Get renderer (for creating new resources)
    Renderer* GetRenderer() const { return m_renderer; }

    // Statistics
    size_t GetShaderCount() const { return m_shaders.size(); }
    size_t GetTextureCount() const { return m_textures.size(); }
    size_t GetBufferCount() const { return m_buffers.size(); }
    size_t GetVertexArrayCount() const { return m_vertexArrays.size(); }
    size_t GetFramebufferCount() const { return m_framebuffers.size(); }

private:
    Renderer* m_renderer;

    // Resource storage with reference counting via shared_ptr
    std::unordered_map<std::string, ShaderHandle> m_shaders;
    std::unordered_map<std::string, TextureHandle> m_textures;
    std::unordered_map<std::string, BufferHandle> m_buffers;
    std::unordered_map<std::string, VertexArrayHandle> m_vertexArrays;
    std::unordered_map<std::string, FramebufferHandle> m_framebuffers;

    // Helper to create deleter functions
    template<typename ResourceType>
    std::function<void(Renderer*, ResourceType*)> CreateDestroyer();
};

} // namespace SmartRenderer
