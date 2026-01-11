#pragma once

#include <string>
#include <vector>

namespace SmartRenderer {

class AssetLoader {
public:
    static std::vector<uint8_t> LoadFile(const std::string& path);
    static std::string LoadTextFile(const std::string& path);
};

} // namespace SmartRenderer
