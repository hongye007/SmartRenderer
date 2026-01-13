#pragma once

#include <vector>
#include <cstdint>

namespace SmartRenderer {

// Texture data container - holds raw image data
struct TextureData {
    int width = 0;
    int height = 0;
    int format = 0; // 0 = RGBA, 1 = RGB
    std::vector<uint8_t> data;
    
    bool IsValid() const {
        return width > 0 && height > 0 && !data.empty();
    }
    
    size_t GetDataSize() const {
        return data.size();
    }
};

// Shader source container
struct ShaderSource {
    std::string vertexSource;
    std::string fragmentSource;
    
    bool IsValid() const {
        return !vertexSource.empty() && !fragmentSource.empty();
    }
};

} // namespace SmartRenderer
