#pragma once

#include "ecs/Component.h"
#include "math/MathTypes.h"
#include "resource/ResourceHandle.h"
#include <map>
#include <string>

namespace SmartRenderer {

// Forward declarations
class Shader;
class Texture;

// Material component - shader and textures
// Uses ResourceHandle for automatic lifetime management
class Material : public Component {
public:
    // Use ResourceHandle for automatic resource management
    ShaderHandle shader;
    Color albedo = Color::White;
    
    // Texture slots - all use ResourceHandle for consistency
    TextureHandle albedoTexture;
    TextureHandle normalTexture;
    TextureHandle metallicRoughnessTexture;
    TextureHandle emissiveTexture;
    
    // Material properties
    float metallic = 0.0f;
    float roughness = 0.5f;
    float emissiveIntensity = 0.0f;
    
    // Custom uniform values
    std::map<std::string, float> floatUniforms;
    std::map<std::string, Vector3> vec3Uniforms;
    std::map<std::string, Vector4> vec4Uniforms;
    
    Material() = default;
    Material(const ShaderHandle& s) : shader(s) {}
    
    // Convenience: check if material is valid (has shader)
    bool IsValid() const { return shader.IsValid(); }
};

} // namespace SmartRenderer
