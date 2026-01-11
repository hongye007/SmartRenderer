#pragma once

#include "ecs/World.h"
#include "rendering/Shader.h"
#include "rendering/Buffer.h"
#include "rendering/VertexArray.h"
#include "resource/Mesh.h"
#include "math/Vector.h"
#include "nlohmann/json.hpp"
#include <string>
#include <memory>
#include <map>

namespace SmartRenderer {

class Renderer;

// ECS Protocol - loads ECS configuration from JSON
class ECSProtocol {
public:
    ECSProtocol(Renderer* renderer);
    ~ECSProtocol();

    // Load ECS configuration from JSON file
    bool LoadFromFile(const std::string& path);
    
    // Load ECS configuration from JSON string
    bool LoadFromJSON(const std::string& jsonStr);
    
    // Get the configured World
    World* GetWorld() { return m_world.get(); }
    
    // Get shader by name
    Shader* GetShader(const std::string& name);
    
    // Get mesh by name
    std::shared_ptr<Mesh> GetMesh(const std::string& name);

private:
    Renderer* m_renderer;
    std::unique_ptr<World> m_world;
    
    // Resource caches
    std::map<std::string, Shader*> m_shaders;
    std::map<std::string, std::shared_ptr<Mesh>> m_meshes;
    
    // Helper methods for parsing JSON
    bool ParseEntities(const nlohmann::json& entitiesJson);
    bool ParseShaders(const nlohmann::json& shadersJson);
    bool ParseMeshes(const nlohmann::json& meshesJson);
    bool ParseSystems(const nlohmann::json& systemsJson);
    
    // Helper to create mesh from geometry data
    std::shared_ptr<Mesh> CreateMeshFromGeometry(const nlohmann::json& geometryJson, const Vector3& offset = Vector3::Zero);
};

} // namespace SmartRenderer
