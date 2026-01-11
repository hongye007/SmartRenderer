#pragma once

#include "ecs/Component.h"
#include "math/MathTypes.h"
#include "math/Vector.h"

namespace SmartRenderer {

// Light component
class Light : public Component {
public:
    enum class Type {
        Directional,
        Point,
        Spot
    };
    
    Type type = Type::Directional;
    Color color = Color::White;
    float intensity = 1.0f;
    
    // Point/Spot light parameters
    float range = 10.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    
    // Spot light parameters
    float innerConeAngle = 30.0f; // degrees
    float outerConeAngle = 45.0f;  // degrees
    
    // Shadow parameters
    bool castShadows = false;
    float shadowBias = 0.005f;
    
    Light() = default;
    Light(Type t, const Color& c, float i) : type(t), color(c), intensity(i) {}
};

} // namespace SmartRenderer
