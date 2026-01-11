#include "ecs/ECSProtocol.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Material.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "ecs/systems/RenderSystem.h"
#include "core/Renderer.h"
#include "resource/AssetLoader.h"
#include "math/Vector.h"
#include "math/Quaternion.h"
#include <fstream>
#include <sstream>

// Use nlohmann/json library for JSON parsing
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace SmartRenderer {

ECSProtocol::ECSProtocol(Renderer* renderer) 
    : m_renderer(renderer) {
    m_world = std::make_unique<World>();
}

ECSProtocol::~ECSProtocol() {
    // Shaders are owned by renderer, meshes are shared_ptr
}

bool ECSProtocol::LoadFromFile(const std::string& path) {
    try {
        std::string jsonStr = AssetLoader::LoadTextFile(path);
        if (jsonStr.empty()) {
            return false;
        }
        return LoadFromJSON(jsonStr);
    } catch (...) {
        return false;
    }
}

bool ECSProtocol::LoadFromJSON(const std::string& jsonStr) {
    if (!m_renderer) {
        return false;
    }
    
    try {
        json root = json::parse(jsonStr);
        
        // Parse shaders first (needed by entities)
        if (root.contains("shaders") && root["shaders"].is_array()) {
            if (!ParseShaders(root["shaders"])) {
                return false;
            }
        }
        
        // Parse meshes (needed by entities)
        if (root.contains("meshes") && root["meshes"].is_array()) {
            if (!ParseMeshes(root["meshes"])) {
                return false;
            }
        }
        
        // Parse systems
        if (root.contains("systems") && root["systems"].is_array()) {
            if (!ParseSystems(root["systems"])) {
                return false;
            }
        }
        
        // Parse entities
        if (root.contains("entities") && root["entities"].is_array()) {
            if (!ParseEntities(root["entities"])) {
                return false;
            }
        }
        
        return true;
    } catch (const json::parse_error& e) {
        #ifdef _DEBUG
        fprintf(stderr, "ECSProtocol: JSON parse error: %s\n", e.what());
        #endif
        return false;
    } catch (const json::type_error& e) {
        #ifdef _DEBUG
        fprintf(stderr, "ECSProtocol: JSON type error: %s\n", e.what());
        #endif
        return false;
    } catch (const std::exception& e) {
        #ifdef _DEBUG
        fprintf(stderr, "ECSProtocol: Error loading JSON: %s\n", e.what());
        #endif
        return false;
    }
}

bool ECSProtocol::ParseShaders(const json& shadersJson) {
    for (const auto& shaderJson : shadersJson) {
        if (!shaderJson.is_object()) continue;
        
        if (!shaderJson.contains("name") || !shaderJson.contains("vertex") || !shaderJson.contains("fragment")) {
            continue;
        }
        
        std::string name = shaderJson["name"].get<std::string>();
        std::string vertexSrc = shaderJson["vertex"].get<std::string>();
        std::string fragmentSrc = shaderJson["fragment"].get<std::string>();
        
        Shader* shader = m_renderer->CreateShader(vertexSrc, fragmentSrc);
        if (shader && shader->IsValid()) {
            m_shaders[name] = shader;
            #ifdef _DEBUG
            fprintf(stderr, "ECSProtocol: Created shader '%s'\n", name.c_str());
            #endif
        } else {
            #ifdef _DEBUG
            fprintf(stderr, "ECSProtocol: Failed to create shader '%s'\n", name.c_str());
            #endif
        }
    }
    return true;
}

bool ECSProtocol::ParseMeshes(const json& meshesJson) {
    for (const auto& meshJson : meshesJson) {
        if (!meshJson.is_object()) continue;
        
        if (!meshJson.contains("name")) {
            continue;
        }
        
        std::string name = meshJson["name"].get<std::string>();
        
        if (meshJson.contains("geometry")) {
            auto mesh = CreateMeshFromGeometry(meshJson["geometry"], Vector3::Zero);
            if (mesh) {
                m_meshes[name] = mesh;
                #ifdef _DEBUG
                fprintf(stderr, "ECSProtocol: Created mesh '%s'\n", name.c_str());
                #endif
            }
        }
    }
    return true;
}

std::shared_ptr<Mesh> ECSProtocol::CreateMeshFromGeometry(const json& geometryJson, const Vector3& offset) {
    if (!geometryJson.is_object()) {
        return nullptr;
    }
    
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    
    if (geometryJson.contains("type") && geometryJson["type"].is_string()) {
        std::string type = geometryJson["type"].get<std::string>();
        
        if (type == "triangle" && geometryJson.contains("vertices") && geometryJson["vertices"].is_array()) {
            std::vector<float> vertices;
            for (const auto& v : geometryJson["vertices"]) {
                if (v.is_number()) {
                    vertices.push_back(v.get<float>());
                }
            }
            
            if (vertices.size() >= 6) { // At least 2D triangle (6 floats)
                // Ensure we have 3D coordinates (pad with z=0 if needed)
                // Apply transform offset to vertex positions
                std::vector<float> vertices3D;
                for (size_t i = 0; i < vertices.size(); i += 2) {
                    vertices3D.push_back(vertices[i] + offset.x);
                    vertices3D.push_back(vertices[i + 1] + offset.y);
                    vertices3D.push_back(0.0f + offset.z); // z = 0 + offset.z
                }
                
                Buffer* vertexBuffer = m_renderer->CreateBuffer(
                    vertices3D.size() * sizeof(float), 
                    vertices3D.data()
                );
                
                VertexArray* vertexArray = m_renderer->CreateVertexArray();
                if (vertexArray && vertexBuffer) {
                    vertexArray->SetVertexBuffer(vertexBuffer, 0, 3, 3 * sizeof(float), 0);
                    mesh->vertexArray = vertexArray;
                    mesh->vertexCount = static_cast<int>(vertices3D.size() / 3);
                }
            }
        }
    }
    
    return mesh;
}

bool ECSProtocol::ParseSystems(const json& systemsJson) {
    for (const auto& systemJson : systemsJson) {
        if (!systemJson.is_object()) continue;
        
        if (!systemJson.contains("type")) {
            continue;
        }
        
        std::string type = systemJson["type"].get<std::string>();
        
        if (type == "RenderSystem") {
            auto* renderSystem = m_world->RegisterSystem<RenderSystem>(
                std::make_unique<RenderSystem>(m_renderer)
            );
            #ifdef _DEBUG
            fprintf(stderr, "ECSProtocol: Registered RenderSystem\n");
            #endif
        }
    }
    return true;
}

bool ECSProtocol::ParseEntities(const json& entitiesJson) {
    for (const auto& entityJson : entitiesJson) {
        if (!entityJson.is_object()) continue;
        
        Entity entity = m_world->CreateEntity();
        
        // Parse Transform component first (needed for mesh offset)
        Vector3 position = Vector3::Zero;
        Vector3 scale = Vector3::One;
        if (entityJson.contains("transform")) {
            const auto& transformJson = entityJson["transform"];
            
            if (transformJson.contains("position") && transformJson["position"].is_array()) {
                auto posArray = transformJson["position"];
                if (posArray.size() >= 3) {
                    position = Vector3(
                        posArray[0].get<float>(),
                        posArray[1].get<float>(),
                        posArray[2].get<float>()
                    );
                }
            }
            
            if (transformJson.contains("scale") && transformJson["scale"].is_array()) {
                auto scaleArray = transformJson["scale"];
                if (scaleArray.size() >= 3) {
                    scale = Vector3(
                        scaleArray[0].get<float>(),
                        scaleArray[1].get<float>(),
                        scaleArray[2].get<float>()
                    );
                }
            }
            
            m_world->AddComponent<Transform>(entity, Transform(position, Quaternion::Identity, scale));
        }
        
        // Parse Camera component
        if (entityJson.contains("camera")) {
            const auto& cameraJson = entityJson["camera"];
            Camera camera;
            
            if (cameraJson.contains("fov") && cameraJson["fov"].is_number()) {
                camera.fov = cameraJson["fov"].get<float>();
            }
            if (cameraJson.contains("aspectRatio") && cameraJson["aspectRatio"].is_number()) {
                camera.aspectRatio = cameraJson["aspectRatio"].get<float>();
            }
            if (cameraJson.contains("nearPlane") && cameraJson["nearPlane"].is_number()) {
                camera.nearPlane = cameraJson["nearPlane"].get<float>();
            }
            if (cameraJson.contains("farPlane") && cameraJson["farPlane"].is_number()) {
                camera.farPlane = cameraJson["farPlane"].get<float>();
            }
            if (cameraJson.contains("isMainCamera") && cameraJson["isMainCamera"].is_boolean()) {
                camera.isMainCamera = cameraJson["isMainCamera"].get<bool>();
            }
            
            camera.UpdateProjectionMatrix();
            m_world->AddComponent<Camera>(entity, std::move(camera));
        }
        
        // Parse Material component
        if (entityJson.contains("material")) {
            const auto& materialJson = entityJson["material"];
            Material material;
            
            if (materialJson.contains("shader") && materialJson["shader"].is_string()) {
                std::string shaderName = materialJson["shader"].get<std::string>();
                auto it = m_shaders.find(shaderName);
                if (it != m_shaders.end()) {
                    material.shader = it->second;
                }
            }
            
            if (materialJson.contains("albedo") && materialJson["albedo"].is_array()) {
                auto albedoArray = materialJson["albedo"];
                if (albedoArray.size() >= 4) {
                    material.albedo = Color(
                        albedoArray[0].get<float>(),
                        albedoArray[1].get<float>(),
                        albedoArray[2].get<float>(),
                        albedoArray[3].get<float>()
                    );
                }
            }
            
            m_world->AddComponent<Material>(entity, std::move(material));
        }
        
        // Parse MeshComponent
        if (entityJson.contains("mesh")) {
            const auto& meshJson = entityJson["mesh"];
            if (meshJson.is_string()) {
                std::string meshName = meshJson.get<std::string>();
                auto it = m_meshes.find(meshName);
                if (it != m_meshes.end()) {
                    MeshComponent meshComp;
                    meshComp.mesh = it->second;
                    m_world->AddComponent<MeshComponent>(entity, std::move(meshComp));
                }
            } else if (meshJson.is_object() && meshJson.contains("geometry")) {
                // Inline geometry definition - apply transform offset
                auto mesh = CreateMeshFromGeometry(meshJson["geometry"], position);
                if (mesh) {
                    MeshComponent meshComp;
                    meshComp.mesh = mesh;
                    m_world->AddComponent<MeshComponent>(entity, std::move(meshComp));
                }
            }
        }
        
        // Parse Light component
        if (entityJson.contains("light")) {
            const auto& lightJson = entityJson["light"];
            Light light;
            
            if (lightJson.contains("type") && lightJson["type"].is_string()) {
                std::string typeStr = lightJson["type"].get<std::string>();
                if (typeStr == "Directional") {
                    light.type = Light::Type::Directional;
                } else if (typeStr == "Point") {
                    light.type = Light::Type::Point;
                } else if (typeStr == "Spot") {
                    light.type = Light::Type::Spot;
                }
            }
            
            if (lightJson.contains("color") && lightJson["color"].is_array()) {
                auto colorArray = lightJson["color"];
                if (colorArray.size() >= 4) {
                    light.color = Color(
                        colorArray[0].get<float>(),
                        colorArray[1].get<float>(),
                        colorArray[2].get<float>(),
                        colorArray[3].get<float>()
                    );
                }
            }
            
            if (lightJson.contains("intensity") && lightJson["intensity"].is_number()) {
                light.intensity = lightJson["intensity"].get<float>();
            }
            
            m_world->AddComponent<Light>(entity, std::move(light));
        }
    }
    
    return true;
}

Shader* ECSProtocol::GetShader(const std::string& name) {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second : nullptr;
}

std::shared_ptr<Mesh> ECSProtocol::GetMesh(const std::string& name) {
    auto it = m_meshes.find(name);
    return (it != m_meshes.end()) ? it->second : nullptr;
}

} // namespace SmartRenderer
