#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace SmartRenderer {

class Shader;
class Texture;
class Mesh;

class Renderer;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    void SetRenderer(Renderer* renderer) { m_renderer = renderer; }

    Shader* LoadShader(const std::string& name,
                      const std::string& vertexPath,
                      const std::string& fragmentPath);
    Texture* LoadTexture(const std::string& path);
    Mesh* LoadMesh(const std::string& path);

    void UnloadShader(const std::string& name);
    void UnloadTexture(const std::string& path);
    void UnloadMesh(const std::string& path);

    void Clear();

private:
    Renderer* m_renderer;
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
};

} // namespace SmartRenderer
