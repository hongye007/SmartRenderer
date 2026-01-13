#pragma once

#include <string>
#include <memory>

namespace SmartRenderer {

// Forward declarations
class Window;
class InputManager;
class FileSystem;

// Platform types
enum class PlatformType {
    Windows,
    macOS,
    iOS,
    Android,
    Linux
};

// Platform configuration
struct PlatformConfig {
    PlatformType type = PlatformType::Windows;
    std::string appName = "SmartRenderer App";
    int windowWidth = 800;
    int windowHeight = 600;
    bool fullscreen = false;
    bool vsync = true;
    int targetFPS = 60;
};

// Abstract platform interface
class Platform {
public:
    virtual ~Platform() = default;

    // Platform lifecycle
    virtual bool Initialize(const PlatformConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual void RunMainLoop() = 0;
    virtual void RequestExit() = 0;

    // Window management
    virtual Window* CreateWindow(int width, int height, const std::string& title) = 0;
    virtual void DestroyWindow(Window* window) = 0;

    // Input management
    virtual InputManager* GetInputManager() = 0;

    // File system
    virtual FileSystem* GetFileSystem() = 0;

    // Platform-specific features
    virtual PlatformType GetType() const = 0;
    virtual std::string GetPlatformName() const = 0;
    virtual bool IsMobile() const = 0;

    // System information
    virtual int GetScreenWidth() const = 0;
    virtual int GetScreenHeight() const = 0;
    virtual float GetScreenDensity() const = 0;

    // File system paths
    virtual std::string GetResourcePath(const std::string& relativePath) const = 0;
    virtual std::string GetWritablePath(const std::string& relativePath) const = 0;

    // Time
    virtual double GetCurrentTime() const = 0;
    virtual double GetTimeSinceStart() const = 0;
};

// Platform factory functions
// Automatically detects current platform
std::unique_ptr<Platform> CreatePlatform();
// Manually specify platform type
std::unique_ptr<Platform> CreatePlatform(PlatformType type);

} // namespace SmartRenderer
