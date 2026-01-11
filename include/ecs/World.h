#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>
#include <algorithm>
#include <iterator>

namespace SmartRenderer {

// ECS World - manages entities, components, and systems
class World {
public:
    World();
    ~World();
    
    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(Entity entity) const;
    
    // Component management
    template<typename T>
    T* AddComponent(Entity entity, T&& component);
    
    template<typename T>
    T* GetComponent(Entity entity);
    
    template<typename T>
    const T* GetComponent(Entity entity) const;
    
    template<typename T>
    bool HasComponent(Entity entity) const;
    
    template<typename T>
    void RemoveComponent(Entity entity);
    
    template<typename... ComponentTypes>
    std::vector<Entity> GetEntitiesWith() const;
    
    // System management
    template<typename T>
    T* RegisterSystem(std::unique_ptr<T> system);
    
    template<typename T>
    T* GetSystem();
    
    template<typename T>
    void UnregisterSystem();
    
    // Update all systems
    void Update(float deltaTime);
    
    // Clear all entities, components, and systems
    void Clear();

private:
    // Component storage: type_index -> (entity -> component)
    std::unordered_map<std::type_index, std::unordered_map<Entity, std::unique_ptr<Component>>> m_components;
    
    // System storage: type_index -> system
    std::unordered_map<std::type_index, std::unique_ptr<System>> m_systems;
    
    // Entity counter
    Entity m_nextEntityId;
    
    // Valid entities
    std::vector<bool> m_validEntities;
    
    // Sorted systems for update order
    std::vector<System*> m_sortedSystems;
    bool m_systemsDirty;
    
    void SortSystems();
};

// Template implementations
template<typename T>
T* World::AddComponent(Entity entity, T&& component) {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    if (!IsValid(entity)) {
        return nullptr;
    }
    
    std::type_index typeId = std::type_index(typeid(T));
    auto& componentMap = m_components[typeId];
    
    auto componentPtr = std::make_unique<T>(std::forward<T>(component));
    T* result = componentPtr.get();
    componentMap[entity] = std::move(componentPtr);
    
    return result;
}

template<typename T>
T* World::GetComponent(Entity entity) {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    if (!IsValid(entity)) {
        return nullptr;
    }
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_components.find(typeId);
    if (it == m_components.end()) {
        return nullptr;
    }
    
    auto componentIt = it->second.find(entity);
    if (componentIt == it->second.end()) {
        return nullptr;
    }
    
    return static_cast<T*>(componentIt->second.get());
}

template<typename T>
const T* World::GetComponent(Entity entity) const {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    if (!IsValid(entity)) {
        return nullptr;
    }
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_components.find(typeId);
    if (it == m_components.end()) {
        return nullptr;
    }
    
    auto componentIt = it->second.find(entity);
    if (componentIt == it->second.end()) {
        return nullptr;
    }
    
    return static_cast<const T*>(componentIt->second.get());
}

template<typename T>
bool World::HasComponent(Entity entity) const {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    if (!IsValid(entity)) {
        return false;
    }
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_components.find(typeId);
    if (it == m_components.end()) {
        return false;
    }
    
    return it->second.find(entity) != it->second.end();
}

template<typename T>
void World::RemoveComponent(Entity entity) {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    if (!IsValid(entity)) {
        return;
    }
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_components.find(typeId);
    if (it != m_components.end()) {
        it->second.erase(entity);
    }
}

template<typename... ComponentTypes>
std::vector<Entity> World::GetEntitiesWith() const {
    std::vector<Entity> result;
    
    // Get entities that have all specified component types
    std::vector<std::vector<Entity>> entityLists;
    (entityLists.push_back([this]() {
        std::type_index typeId = std::type_index(typeid(ComponentTypes));
        auto it = m_components.find(typeId);
        std::vector<Entity> entities;
        if (it != m_components.end()) {
            for (const auto& pair : it->second) {
                if (IsValid(pair.first)) {
                    entities.push_back(pair.first);
                }
            }
        }
        return entities;
    }()), ...);
    
    // Find intersection (entities that have all components)
    if (entityLists.empty()) {
        return result;
    }
    
    // Start with first list
    result = entityLists[0];
    
    // Intersect with remaining lists
    for (size_t i = 1; i < entityLists.size(); ++i) {
        std::vector<Entity> intersection;
        std::set_intersection(
            result.begin(), result.end(),
            entityLists[i].begin(), entityLists[i].end(),
            std::back_inserter(intersection)
        );
        result = intersection;
    }
    
    return result;
}

template<typename T>
T* World::RegisterSystem(std::unique_ptr<T> system) {
    static_assert(std::is_base_of_v<System, T>, "T must inherit from System");
    
    std::type_index typeId = std::type_index(typeid(T));
    T* result = system.get();
    m_systems[typeId] = std::move(system);
    m_systemsDirty = true;
    
    result->OnRegister(*this);
    return result;
}

template<typename T>
T* World::GetSystem() {
    static_assert(std::is_base_of_v<System, T>, "T must inherit from System");
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_systems.find(typeId);
    if (it == m_systems.end()) {
        return nullptr;
    }
    
    return static_cast<T*>(it->second.get());
}

template<typename T>
void World::UnregisterSystem() {
    static_assert(std::is_base_of_v<System, T>, "T must inherit from System");
    
    std::type_index typeId = std::type_index(typeid(T));
    auto it = m_systems.find(typeId);
    if (it != m_systems.end()) {
        it->second->OnUnregister(*this);
        m_systems.erase(it);
        m_systemsDirty = true;
    }
}

} // namespace SmartRenderer
