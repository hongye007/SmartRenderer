#pragma once

#include <string>
#include <vector>
#include <memory>

namespace SmartRenderer {

// File system interface
class FileSystem {
public:
    virtual ~FileSystem() = default;

    // File operations
    virtual bool FileExists(const std::string& path) const = 0;
    virtual std::vector<uint8_t> ReadFile(const std::string& path) const = 0;
    virtual bool WriteFile(const std::string& path, const std::vector<uint8_t>& data) const = 0;
    virtual bool DeleteFile(const std::string& path) const = 0;

    // Directory operations
    virtual bool DirectoryExists(const std::string& path) const = 0;
    virtual bool CreateDirectory(const std::string& path) const = 0;
    virtual bool DeleteDirectory(const std::string& path) const = 0;
    virtual std::vector<std::string> ListDirectory(const std::string& path) const = 0;

    // Path operations
    virtual std::string GetAbsolutePath(const std::string& relativePath) const = 0;
    virtual std::string GetDirectory(const std::string& path) const = 0;
    virtual std::string GetFileName(const std::string& path) const = 0;
    virtual std::string GetExtension(const std::string& path) const = 0;
    virtual std::string CombinePaths(const std::string& path1, const std::string& path2) const = 0;
};

} // namespace SmartRenderer
