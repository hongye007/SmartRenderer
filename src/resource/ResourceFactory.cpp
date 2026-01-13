#include "resource/ResourceFactory.h"
#include "core/Renderer.h"
#include "rendering/Texture.h"
#include "rendering/Shader.h"
#include "rendering/Buffer.h"
#include "rendering/VertexArray.h"
#include "rendering/Framebuffer.h"
#include <algorithm>

namespace SmartRenderer {

ResourceFactory::ResourceFactory(Renderer* renderer, ResourceManager* dataManager)
    : m_registry(renderer)
    , m_externalDataManager(dataManager)
    , m_renderer(renderer) {
}

ResourceFactory::~ResourceFactory() {
    // Clear resources before destroying
    // Invalidate renderer first to prevent access after destruction
    m_registry.InvalidateRenderer();
    Clear();
}

TextureHandle ResourceFactory::LoadTexture(const std::string& path, const std::string& name) {
    if (!m_renderer) {
        return TextureHandle();
    }

    // Use provided name or generate from path
    std::string resourceName = name.empty() ? GenerateNameFromPath(path) : name;

    // Check if already loaded
    if (m_registry.HasTexture(resourceName)) {
        return m_registry.GetTexture(resourceName);
    }

    // Load texture data
    ResourceManager* dataMgr = m_externalDataManager ? m_externalDataManager : &m_dataManager;
    TextureData textureData;
    if (!dataMgr->LoadTextureData(path, textureData)) {
        return TextureHandle();
    }

    // Create GPU texture
    Texture* texture = m_renderer->CreateTexture(
        textureData.width,
        textureData.height,
        textureData.format,
        textureData.data.data()
    );

    if (!texture) {
        return TextureHandle();
    }

    // Register and return handle
    return m_registry.RegisterTexture(resourceName, texture);
}

ShaderHandle ResourceFactory::LoadShader(const std::string& name,
                                        const std::string& vertexPath,
                                        const std::string& fragmentPath) {
    if (!m_renderer) {
        return ShaderHandle();
    }

    // Check if already loaded
    if (m_registry.HasShader(name)) {
        return m_registry.GetShader(name);
    }

    // Load shader source
    ResourceManager* dataMgr = m_externalDataManager ? m_externalDataManager : &m_dataManager;
    ShaderSource shaderSource;
    if (!dataMgr->LoadShaderSource(name, vertexPath, fragmentPath, shaderSource)) {
        return ShaderHandle();
    }

    // Create GPU shader
    Shader* shader = m_renderer->CreateShader(
        shaderSource.vertexSource,
        shaderSource.fragmentSource
    );

    if (!shader || !shader->IsValid()) {
        if (shader) {
            m_renderer->DestroyShader(shader);
        }
        return ShaderHandle();
    }

    // Register and return handle
    return m_registry.RegisterShader(name, shader);
}

TextureHandle ResourceFactory::CreateTexture(const std::string& name,
                                             int width, int height, int format,
                                             const void* data) {
    if (!m_renderer) {
        return TextureHandle();
    }

    // Check if already exists
    if (m_registry.HasTexture(name)) {
        return m_registry.GetTexture(name);
    }

    // Create GPU texture
    Texture* texture = m_renderer->CreateTexture(width, height, format, data);
    if (!texture) {
        return TextureHandle();
    }

    // Register and return handle
    return m_registry.RegisterTexture(name, texture);
}

ShaderHandle ResourceFactory::CreateShader(const std::string& name,
                                          const std::string& vertexSource,
                                          const std::string& fragmentSource) {
    if (!m_renderer) {
        return ShaderHandle();
    }

    // Check if already exists
    if (m_registry.HasShader(name)) {
        return m_registry.GetShader(name);
    }

    // Create GPU shader
    Shader* shader = m_renderer->CreateShader(vertexSource, fragmentSource);
    if (!shader || !shader->IsValid()) {
        if (shader) {
            m_renderer->DestroyShader(shader);
        }
        return ShaderHandle();
    }

    // Register and return handle
    return m_registry.RegisterShader(name, shader);
}

TextureHandle ResourceFactory::GetTexture(const std::string& name) const {
    return m_registry.GetTexture(name);
}

ShaderHandle ResourceFactory::GetShader(const std::string& name) const {
    return m_registry.GetShader(name);
}

BufferHandle ResourceFactory::GetBuffer(const std::string& name) const {
    return m_registry.GetBuffer(name);
}

VertexArrayHandle ResourceFactory::GetVertexArray(const std::string& name) const {
    return m_registry.GetVertexArray(name);
}

FramebufferHandle ResourceFactory::GetFramebuffer(const std::string& name) const {
    return m_registry.GetFramebuffer(name);
}

bool ResourceFactory::HasTexture(const std::string& name) const {
    return m_registry.HasTexture(name);
}

bool ResourceFactory::HasShader(const std::string& name) const {
    return m_registry.HasShader(name);
}

TextureHandle ResourceFactory::RegisterTexture(const std::string& name, Texture* texture) {
    return m_registry.RegisterTexture(name, texture);
}

ShaderHandle ResourceFactory::RegisterShader(const std::string& name, Shader* shader) {
    return m_registry.RegisterShader(name, shader);
}

BufferHandle ResourceFactory::RegisterBuffer(const std::string& name, Buffer* buffer) {
    return m_registry.RegisterBuffer(name, buffer);
}

VertexArrayHandle ResourceFactory::RegisterVertexArray(const std::string& name, VertexArray* vao) {
    return m_registry.RegisterVertexArray(name, vao);
}

FramebufferHandle ResourceFactory::RegisterFramebuffer(const std::string& name, Framebuffer* framebuffer) {
    return m_registry.RegisterFramebuffer(name, framebuffer);
}

void ResourceFactory::Clear() {
    // Clear registry first (destroys all GPU resources)
    m_registry.Clear();
    
    // Clear data manager cache (only if using internal manager)
    if (!m_externalDataManager) {
        m_dataManager.Clear();
    }
    
    // Note: Don't invalidate renderer here, as it might still be needed
    // Invalidation should be done explicitly before renderer shutdown
}

std::string ResourceFactory::GenerateNameFromPath(const std::string& path) const {
    // Extract filename without extension as default name
    std::string name = path;
    
    // Remove directory path
    size_t lastSlash = name.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        name = name.substr(lastSlash + 1);
    }
    
    // Remove extension
    size_t lastDot = name.find_last_of(".");
    if (lastDot != std::string::npos) {
        name = name.substr(0, lastDot);
    }
    
    return name;
}

} // namespace SmartRenderer
