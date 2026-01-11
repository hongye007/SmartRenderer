#pragma once

#include "ecs/Component.h"
#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"

namespace SmartRenderer {

// Transform component - position, rotation, scale
class Transform : public Component {
public:
    Vector3 position = Vector3::Zero;
    Quaternion rotation = Quaternion::Identity;
    Vector3 scale = Vector3::One;
    
    Transform() = default;
    Transform(const Vector3& pos) : position(pos) {}
    Transform(const Vector3& pos, const Quaternion& rot) : position(pos), rotation(rot) {}
    Transform(const Vector3& pos, const Quaternion& rot, const Vector3& scl) 
        : position(pos), rotation(rot), scale(scl) {}
    
    // Get transformation matrix
    Matrix4 GetMatrix() const;
    
    // Get forward, right, up vectors
    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;
};

} // namespace SmartRenderer
