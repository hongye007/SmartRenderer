#include "ecs/components/Transform.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"
#include "math/Vector.h"

namespace SmartRenderer {

Matrix4 Transform::GetMatrix() const {
    Matrix4 translation = Matrix4::CreateTranslation(position);
    Matrix4 rotation = Matrix4::CreateRotation(this->rotation);
    Matrix4 scale = Matrix4::CreateScale(this->scale);
    
    // T * R * S
    return translation * rotation * scale;
}

Vector3 Transform::GetForward() const {
    return rotation * Vector3::Forward;
}

Vector3 Transform::GetRight() const {
    return rotation * Vector3::Right;
}

Vector3 Transform::GetUp() const {
    return rotation * Vector3::Up;
}

} // namespace SmartRenderer
