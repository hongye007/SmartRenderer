#pragma once

#include "ecs/Component.h"
#include "math/MathTypes.h"
#include "math/Matrix.h"

namespace SmartRenderer {

// Camera component
class Camera : public Component {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };
    
    ProjectionType projectionType = ProjectionType::Perspective;
    
    // Perspective parameters
    float fov = 60.0f; // degrees
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    // Orthographic parameters
    float orthoSize = 10.0f;
    
    // View matrix (calculated from Transform)
    Matrix4 viewMatrix = Matrix4::Identity;
    
    // Projection matrix
    Matrix4 projectionMatrix = Matrix4::Identity;
    
    // Combined view-projection matrix
    Matrix4 viewProjectionMatrix = Matrix4::Identity;
    
    bool isMainCamera = false;
    
    Camera() = default;
    
    // Update projection matrix based on projection type
    void UpdateProjectionMatrix();
    
    // Update view matrix from transform
    void UpdateViewMatrix(const Matrix4& transformMatrix);
};

} // namespace SmartRenderer
