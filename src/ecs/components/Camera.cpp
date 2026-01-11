#include "ecs/components/Camera.h"
#include "math/Matrix.h"

namespace SmartRenderer {

void Camera::UpdateProjectionMatrix() {
    if (projectionType == ProjectionType::Perspective) {
        projectionMatrix = Matrix4::CreatePerspective(
            fov * PI / 180.0f, // Convert to radians
            aspectRatio,
            nearPlane,
            farPlane
        );
    } else {
        float halfSize = orthoSize * 0.5f;
        projectionMatrix = Matrix4::CreateOrthographic(
            -halfSize * aspectRatio, halfSize * aspectRatio,
            -halfSize, halfSize,
            nearPlane,
            farPlane
        );
    }
    
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Camera::UpdateViewMatrix(const Matrix4& transformMatrix) {
    // Extract position and rotation from transform matrix
    // For now, assume transform matrix is T * R * S
    // View matrix is inverse of transform
    viewMatrix = transformMatrix.Inverse();
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

} // namespace SmartRenderer
