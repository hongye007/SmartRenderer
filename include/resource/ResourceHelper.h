#pragma once

#include "ResourceManager.h"
#include "core/Renderer.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include <iostream>
#include <string>

namespace SmartRenderer {

// Helper utilities for creating GPU resources from ResourceManager data
// This is a convenience layer - ResourceManager itself remains decoupled
class ResourceHelper {
public:
    // Create shader from ResourceManager cache
    static Shader* CreateShaderFromCache(ResourceManager& resourceManager,
                                        const std::string& name,
                                        Renderer* renderer) {
        if (!renderer) {
            std::cerr << "ResourceHelper: Renderer is null" << std::endl;
            return nullptr;
        }

        const ShaderSource* source = resourceManager.GetShaderSource(name);
        if (!source) {
            std::cerr << "ResourceHelper: Shader source not found: " << name << std::endl;
            return nullptr;
        }

        Shader* shader = renderer->CreateShader(source->vertexSource, source->fragmentSource);
        if (!shader || !shader->IsValid()) {
            std::cerr << "ResourceHelper: Failed to compile shader: " << name << std::endl;
            if (shader) {
                renderer->DestroyShader(shader);
            }
            return nullptr;
        }

        return shader;
    }

    // Create texture from ResourceManager cache
    static Texture* CreateTextureFromCache(ResourceManager& resourceManager,
                                          const std::string& path,
                                          Renderer* renderer) {
        if (!renderer) {
            std::cerr << "ResourceHelper: Renderer is null" << std::endl;
            return nullptr;
        }

        const TextureData* data = resourceManager.GetTextureData(path);
        if (!data || !data->IsValid()) {
            std::cerr << "ResourceHelper: Texture data not found or invalid: " << path << std::endl;
            return nullptr;
        }

        Texture* texture = renderer->CreateTexture(
            data->width,
            data->height,
            data->format,
            data->data.data()
        );

        if (!texture) {
            std::cerr << "ResourceHelper: Failed to create texture from cached data: " << path << std::endl;
            return nullptr;
        }

        return texture;
    }

    // Convenience: Load and create shader in one call
    static Shader* LoadShader(ResourceManager& resourceManager,
                             const std::string& name,
                             const std::string& vertexPath,
                             const std::string& fragmentPath,
                             Renderer* renderer) {
        ShaderSource source;
        if (!resourceManager.LoadShaderSource(name, vertexPath, fragmentPath, source)) {
            return nullptr;
        }

        return CreateShaderFromCache(resourceManager, name, renderer);
    }

    // Convenience: Load and create texture in one call
    static Texture* LoadTexture(ResourceManager& resourceManager,
                               const std::string& path,
                               Renderer* renderer) {
        TextureData data;
        if (!resourceManager.LoadTextureData(path, data)) {
            return nullptr;
        }

        return CreateTextureFromCache(resourceManager, path, renderer);
    }
};

} // namespace SmartRenderer
