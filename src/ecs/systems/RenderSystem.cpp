#include "ecs/systems/RenderSystem.h"
#include "ecs/World.h"
#include "ecs/Entity.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Material.h"
#include "ecs/components/Camera.h"
#include "rendering/Shader.h"
#include "rendering/VertexArray.h"
#include "rendering/Texture.h"
#include "math/Matrix.h"
#include "math/MathTypes.h"

namespace SmartRenderer {

RenderSystem::RenderSystem() 
    : m_activeCamera(INVALID_ENTITY) {
}

void RenderSystem::Update(World& world, float deltaTime) {
    // Clear command queue for this frame
    m_commandQueue.Clear();
    
    // Find active camera
    if (m_activeCamera == INVALID_ENTITY) {
        // Try to find main camera
        auto cameras = world.GetEntitiesWith<Camera>();
        for (Entity camEntity : cameras) {
            Camera* cam = world.GetComponent<Camera>(camEntity);
            if (cam && cam->isMainCamera) {
                m_activeCamera = camEntity;
                break;
            }
        }
        
        // If no main camera, use first camera
        if (m_activeCamera == INVALID_ENTITY && !cameras.empty()) {
            m_activeCamera = cameras[0];
        }
    }
    
    // Get camera view-projection matrix
    Matrix4 viewProjMatrix = Matrix4::Identity;
    if (m_activeCamera != INVALID_ENTITY) {
        Camera* camera = world.GetComponent<Camera>(m_activeCamera);
        if (camera) {
            // Update camera matrices if needed
            Transform* camTransform = world.GetComponent<Transform>(m_activeCamera);
            if (camTransform) {
                camera->UpdateViewMatrix(camTransform->GetMatrix());
            }
            camera->UpdateProjectionMatrix();
            viewProjMatrix = camera->viewProjectionMatrix;
        }
    }
    
    // Render all entities with Transform, Mesh, and Material
    auto renderableEntities = world.GetEntitiesWith<Transform, MeshComponent, Material>();
    
    for (Entity entity : renderableEntities) {
        RenderEntity(world, entity, viewProjMatrix);
    }
}

void RenderSystem::RenderEntity(World& world, Entity entity, const Matrix4& viewProjMatrix) {
    Transform* transform = world.GetComponent<Transform>(entity);
    MeshComponent* meshComp = world.GetComponent<MeshComponent>(entity);
    Material* material = world.GetComponent<Material>(entity);
    
    if (!transform || !meshComp || !meshComp->mesh || !material || !material->shader) {
        return;
    }
    
    // Get model matrix
    Matrix4 modelMatrix = transform->GetMatrix();
    Matrix4 mvpMatrix = viewProjMatrix * modelMatrix;
    
    // Generate render commands (decoupled from Renderer)
    m_commandQueue.PushBindShader(material->shader);
    
    // Set uniforms via commands
    if (material->shader && material->shader->IsValid()) {
        // Material color (for simplified 2D shader)
        m_commandQueue.PushSetUniformVec4(material->shader, "uColor",
            material->albedo.r,
            material->albedo.g,
            material->albedo.b,
            material->albedo.a);
        
        // Material properties
        m_commandQueue.PushSetUniformFloat(material->shader, "uMetallic", material->metallic);
        m_commandQueue.PushSetUniformFloat(material->shader, "uRoughness", material->roughness);
        
        // MVP matrix
        m_commandQueue.PushSetUniformMatrix4(material->shader, "uMVP", mvpMatrix);
    } else {
        return;
    }
    
    // Textures
    if (material->albedoTexture) {
        m_commandQueue.PushBindTexture(material->albedoTexture.get(), 0);
        m_commandQueue.PushSetUniformInt(material->shader, "uAlbedoTexture", 0);
    }
    
    // Bind vertex array and draw
    if (meshComp->mesh && meshComp->mesh->vertexArray) {
        m_commandQueue.PushBindVertexArray(meshComp->mesh->vertexArray);
        
        if (meshComp->mesh->indexBuffer && meshComp->mesh->indexCount > 0) {
            m_commandQueue.PushDrawElements(meshComp->mesh->indexCount, 0);
        } else if (meshComp->mesh->vertexCount > 0) {
            m_commandQueue.PushDrawArrays(0, meshComp->mesh->vertexCount);
        }
    }
}

void RenderSystem::SetActiveCamera(Entity cameraEntity) {
    m_activeCamera = cameraEntity;
}

} // namespace SmartRenderer
