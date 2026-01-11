#include "platform/windows/WindowsFileSystem.h"
#include "platform/FileSystem.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstdint>

// Undefine Windows macros that conflict with our method names
#ifdef DeleteFile
#undef DeleteFile
#endif
#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace SmartRenderer {

bool WindowsFileSystem::FileExists(const std::string& path) const {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }

std::vector<uint8_t> WindowsFileSystem::ReadFile(const std::string& path) const {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }

        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

bool WindowsFileSystem::WriteFile(const std::string& path, const std::vector<uint8_t>& data) const {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }

bool WindowsFileSystem::DeleteFile(const std::string& path) const {
        return std::filesystem::remove(path);
    }

bool WindowsFileSystem::DirectoryExists(const std::string& path) const {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }

bool WindowsFileSystem::CreateDirectory(const std::string& path) const {
        return std::filesystem::create_directories(path);
    }

bool WindowsFileSystem::DeleteDirectory(const std::string& path) const {
        return std::filesystem::remove_all(path) > 0;
    }

std::vector<std::string> WindowsFileSystem::ListDirectory(const std::string& path) const {
        std::vector<std::string> files;
        if (!DirectoryExists(path)) {
            return files;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(entry.path().string());
        }
        return files;
    }

std::string WindowsFileSystem::GetAbsolutePath(const std::string& relativePath) const {
        return std::filesystem::absolute(relativePath).string();
    }

std::string WindowsFileSystem::GetDirectory(const std::string& path) const {
        return std::filesystem::path(path).parent_path().string();
    }

std::string WindowsFileSystem::GetFileName(const std::string& path) const {
        return std::filesystem::path(path).filename().string();
    }

std::string WindowsFileSystem::GetExtension(const std::string& path) const {
        return std::filesystem::path(path).extension().string();
    }

std::string WindowsFileSystem::CombinePaths(const std::string& path1, const std::string& path2) const {
    return (std::filesystem::path(path1) / path2).string();
}

} // namespace SmartRenderer
