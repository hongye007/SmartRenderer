#pragma once

#include "ecs/System.h"
#include "ecs/Entity.h"
#include "math/Matrix.h"
#include <vector>

namespace SmartRenderer {

class World;
class Renderer;

// Render system - renders entities with Transform, Mesh, and Material components
class RenderSystem : public System {
public:
    RenderSystem(Renderer* renderer);
    
    void Update(World& world, float deltaTime) override;
    
    int GetPriority() const override { return 100; } // Render after other systems
    
    // Set the active camera (or find main camera)
    void SetActiveCamera(Entity cameraEntity);
    Entity GetActiveCamera() const { return m_activeCamera; }
    
private:
    Renderer* m_renderer;
    Entity m_activeCamera;
    
    void RenderEntity(World& world, Entity entity, const Matrix4& viewProjMatrix);
};

} // namespace SmartRenderer
