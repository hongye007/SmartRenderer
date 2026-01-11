#pragma once

#include "ecs/Component.h"
#include "resource/Mesh.h"
#include <memory>

namespace SmartRenderer {

// Mesh component - reference to mesh data
class MeshComponent : public Component {
public:
    std::shared_ptr<Mesh> mesh;
    
    MeshComponent() = default;
    MeshComponent(std::shared_ptr<Mesh> m) : mesh(m) {}
};

} // namespace SmartRenderer
