#pragma once

#include "Entity.h"
#include <vector>

namespace SmartRenderer {

class World;

// Base class for all systems
// Systems contain logic that operates on entities with specific components
class System {
public:
    virtual ~System() = default;
    
    // Called once per frame
    virtual void Update(World& world, float deltaTime) = 0;
    
    // Called when system is registered with world
    virtual void OnRegister(World& world) {}
    
    // Called when system is unregistered from world
    virtual void OnUnregister(World& world) {}
    
    // Get priority for update order (lower = earlier)
    virtual int GetPriority() const { return 0; }
    
protected:
    // Helper to get all entities with specific component types
    template<typename... ComponentTypes>
    std::vector<Entity> GetEntitiesWith(World& world);
};

} // namespace SmartRenderer
