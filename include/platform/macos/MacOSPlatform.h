#pragma once

#include "platform/Platform.h"

namespace SmartRenderer {

class MacOSPlatform : public Platform {
public:
    MacOSPlatform();
    ~MacOSPlatform() override;

    bool Initialize(const PlatformConfig& config) override;
    void Shutdown() override;
    void RunMainLoop() override;
    void RequestExit() override;
    Window* CreateWindow(int width, int height, const std::string& title) override;
    void DestroyWindow(Window* window) override;
    InputManager* GetInputManager() override;
    FileSystem* GetFileSystem() override;
    PlatformType GetType() const override;
    std::string GetPlatformName() const override;
    bool IsMobile() const override;
    int GetScreenWidth() const override;
    int GetScreenHeight() const override;
    float GetScreenDensity() const override;
    std::string GetResourcePath(const std::string& relativePath) const override;
    std::string GetWritablePath(const std::string& relativePath) const override;
    double GetCurrentTime() const override;
    double GetTimeSinceStart() const override;

private:
    PlatformConfig m_config;
    bool m_initialized;
    double m_startTime;
};

} // namespace SmartRenderer
