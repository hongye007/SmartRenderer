# ECS (Entity Component System) Guide

## Overview

SmartRenderer now includes an ECS (Entity Component System) framework for managing game objects and rendering entities. The ECS architecture separates data (components) from behavior (systems), providing a flexible and performant way to organize your rendering code.

## Core Concepts

### Entity
An Entity is simply a unique identifier (uint32_t). It has no data or behavior - it's just an ID used to group components together.

```cpp
Entity entity = world.CreateEntity();
```

### Component
Components are pure data structures that inherit from `Component`. They contain no logic, only data.

```cpp
class Transform : public Component {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
};
```

### System
Systems contain logic that operates on entities with specific component combinations. They are updated once per frame.

```cpp
class RenderSystem : public System {
    void Update(World& world, float deltaTime) override {
        // Process entities with Transform, Mesh, and Material components
    }
};
```

## Usage Example

```cpp
#include "ecs/World.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Material.h"
#include "ecs/components/Camera.h"
#include "ecs/systems/RenderSystem.h"

// Create world
World world;

// Register systems
auto* renderSystem = world.RegisterSystem<RenderSystem>(
    std::make_unique<RenderSystem>(renderer)
);

// Create camera entity
Entity camera = world.CreateEntity();
world.AddComponent<Transform>(camera, Transform(Vector3(0, 0, 5)));
Camera cam;
cam.isMainCamera = true;
cam.fov = 60.0f;
world.AddComponent<Camera>(camera, std::move(cam));

// Create renderable entity
Entity triangle = world.CreateEntity();
world.AddComponent<Transform>(triangle, Transform(Vector3::Zero));
world.AddComponent<MeshComponent>(triangle, MeshComponent(mesh));
world.AddComponent<Material>(triangle, Material(shader));

// Update loop
while (running) {
    world.Update(deltaTime);
    renderer->Present();
}
```

## Available Components

### Transform
- **Position**: `Vector3 position`
- **Rotation**: `Quaternion rotation`
- **Scale**: `Vector3 scale`
- **Methods**: `GetMatrix()`, `GetForward()`, `GetRight()`, `GetUp()`

### MeshComponent
- **Mesh**: `std::shared_ptr<Mesh> mesh`
- References mesh geometry data (vertices, indices, vertex arrays)

### Material
- **Shader**: `Shader* shader`
- **Albedo**: `Color albedo`
- **Textures**: `albedoTexture`, `normalTexture`, `metallicRoughnessTexture`, `emissiveTexture`
- **Properties**: `metallic`, `roughness`, `emissiveIntensity`

### Camera
- **Projection Type**: `ProjectionType` (Perspective/Orthographic)
- **Parameters**: `fov`, `aspectRatio`, `nearPlane`, `farPlane`
- **Matrices**: `viewMatrix`, `projectionMatrix`, `viewProjectionMatrix`
- **Flag**: `isMainCamera`

### Light
- **Type**: `Type` (Directional/Point/Spot)
- **Color**: `Color color`
- **Intensity**: `float intensity`
- **Parameters**: `range`, `constant`, `linear`, `quadratic` (for attenuation)
- **Spot Parameters**: `innerConeAngle`, `outerConeAngle`
- **Shadow**: `castShadows`, `shadowBias`

## Available Systems

### RenderSystem
Renders entities with `Transform`, `MeshComponent`, and `Material` components.

- Automatically finds and uses the main camera
- Calculates MVP matrices
- Binds shaders and textures
- Draws meshes

## Integration with Render Graph

The ECS system works alongside the Render Graph system:

- **ECS**: Manages scene entities (objects, cameras, lights)
- **Render Graph**: Manages rendering pipeline (passes, post-processing)

You can use ECS to populate render targets that are then processed by the Render Graph, or use Render Graph nodes that query the ECS World for entities to render.

## Best Practices

1. **Component Design**: Keep components small and focused. Each component should represent a single aspect of an entity.

2. **System Priority**: Use `GetPriority()` to control update order. Lower values update first.

3. **Entity Lifecycle**: Always destroy entities when done:
   ```cpp
   world.DestroyEntity(entity);
   ```

4. **Component Access**: Cache component pointers if accessing multiple times:
   ```cpp
   Transform* transform = world.GetComponent<Transform>(entity);
   if (transform) {
       // Use transform multiple times
   }
   ```

5. **Query Optimization**: Use `GetEntitiesWith<...>()` to efficiently find entities with specific component combinations.

## Future Enhancements

- Component pooling for better performance
- Archetype-based storage for cache-friendly access
- Event system for component changes
- Serialization support for saving/loading scenes
- More rendering systems (shadow mapping, culling, etc.)
