#pragma once

#include "platform/FileSystem.h"

namespace SmartRenderer {

class WindowsFileSystem : public FileSystem {
public:
    WindowsFileSystem() = default;
    ~WindowsFileSystem() override = default;

    bool FileExists(const std::string& path) const override;
    std::vector<uint8_t> ReadFile(const std::string& path) const override;
    bool WriteFile(const std::string& path, const std::vector<uint8_t>& data) const override;
    bool DeleteFile(const std::string& path) const override;

    bool DirectoryExists(const std::string& path) const override;
    bool CreateDirectory(const std::string& path) const override;
    bool DeleteDirectory(const std::string& path) const override;
    std::vector<std::string> ListDirectory(const std::string& path) const override;

    std::string GetAbsolutePath(const std::string& relativePath) const override;
    std::string GetDirectory(const std::string& path) const override;
    std::string GetFileName(const std::string& path) const override;
    std::string GetExtension(const std::string& path) const override;
    std::string CombinePaths(const std::string& path1, const std::string& path2) const override;
};

} // namespace SmartRenderer
