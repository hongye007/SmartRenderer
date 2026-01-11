#pragma once

#include "ecs/Component.h"
#include "math/MathTypes.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include <memory>
#include <map>
#include <string>

namespace SmartRenderer {

// Material component - shader and textures
class Material : public Component {
public:
    Shader* shader = nullptr;
    Color albedo = Color::White;
    
    // Texture slots
    std::shared_ptr<Texture> albedoTexture;
    std::shared_ptr<Texture> normalTexture;
    std::shared_ptr<Texture> metallicRoughnessTexture;
    std::shared_ptr<Texture> emissiveTexture;
    
    // Material properties
    float metallic = 0.0f;
    float roughness = 0.5f;
    float emissiveIntensity = 0.0f;
    
    // Custom uniform values
    std::map<std::string, float> floatUniforms;
    std::map<std::string, Vector3> vec3Uniforms;
    std::map<std::string, Vector4> vec4Uniforms;
    
    Material() = default;
    Material(Shader* s) : shader(s) {}
};

} // namespace SmartRenderer
