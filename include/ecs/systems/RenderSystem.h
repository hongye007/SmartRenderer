#pragma once

#include "ecs/System.h"
#include "ecs/Entity.h"
#include "core/RenderCommand.h"
#include "math/Matrix.h"
#include <vector>
#include <memory>

namespace SmartRenderer {

class World;

// Render system - renders entities with Transform, Mesh, and Material components
// Uses command pattern to decouple from Renderer
class RenderSystem : public System {
public:
    RenderSystem();
    
    void Update(World& world, float deltaTime) override;
    
    int GetPriority() const override { return 100; } // Render after other systems
    
    // Set the active camera (or find main camera)
    void SetActiveCamera(Entity cameraEntity);
    Entity GetActiveCamera() const { return m_activeCamera; }
    
    // Get command queue (for execution by Renderer or CommandExecutor)
    RenderCommandQueue& GetCommandQueue() { return m_commandQueue; }
    const RenderCommandQueue& GetCommandQueue() const { return m_commandQueue; }
    
private:
    Entity m_activeCamera;
    RenderCommandQueue m_commandQueue;
    
    void RenderEntity(World& world, Entity entity, const Matrix4& viewProjMatrix);
};

} // namespace SmartRenderer
