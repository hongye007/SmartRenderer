#pragma once

#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"
#include <memory>

namespace SmartRenderer {

class Mesh {
public:
    virtual ~Mesh() = default;
    
    // Geometry data
    VertexArray* vertexArray = nullptr;
    Buffer* indexBuffer = nullptr;
    int vertexCount = 0;
    int indexCount = 0;
    
    // Bounding box (optional)
    // Vector3 minBounds;
    // Vector3 maxBounds;
};

} // namespace SmartRenderer
