#include "ecs/World.h"
#include "ecs/System.h"
#include <algorithm>

namespace SmartRenderer {

World::World() : m_nextEntityId(1), m_systemsDirty(false) {
}

World::~World() {
    Clear();
}

Entity World::CreateEntity() {
    Entity entity = m_nextEntityId++;
    
    // Ensure valid entities vector is large enough
    if (entity >= m_validEntities.size()) {
        m_validEntities.resize(entity + 1, false);
    }
    
    m_validEntities[entity] = true;
    return entity;
}

void World::DestroyEntity(Entity entity) {
    if (!IsValid(entity)) {
        return;
    }
    
    // Remove all components for this entity
    for (auto& [typeId, componentMap] : m_components) {
        componentMap.erase(entity);
    }
    
    // Mark entity as invalid
    m_validEntities[entity] = false;
}

bool World::IsValid(Entity entity) const {
    if (entity == INVALID_ENTITY || entity >= m_validEntities.size()) {
        return false;
    }
    return m_validEntities[entity];
}

void World::Update(float deltaTime) {
    if (m_systemsDirty) {
        SortSystems();
    }
    
    for (System* system : m_sortedSystems) {
        system->Update(*this, deltaTime);
    }
}

void World::Clear() {
    // Unregister all systems
    for (auto& [typeId, system] : m_systems) {
        system->OnUnregister(*this);
    }
    
    m_systems.clear();
    m_components.clear();
    m_sortedSystems.clear();
    m_validEntities.clear();
    m_nextEntityId = 1;
    m_systemsDirty = false;
}

void World::SortSystems() {
    m_sortedSystems.clear();
    m_sortedSystems.reserve(m_systems.size());
    
    for (auto& [typeId, system] : m_systems) {
        m_sortedSystems.push_back(system.get());
    }
    
    std::sort(m_sortedSystems.begin(), m_sortedSystems.end(),
        [](System* a, System* b) {
            return a->GetPriority() < b->GetPriority();
        });
    
    m_systemsDirty = false;
}

} // namespace SmartRenderer
