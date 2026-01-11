#include "platform/ios/IOSPlatform.h"
#include "platform/Platform.h"
#include <chrono>
#include <memory>

namespace SmartRenderer {

IOSPlatform::IOSPlatform()
    : m_initialized(false), m_startTime(0.0) {
}

IOSPlatform::~IOSPlatform() {
    Shutdown();
}

bool IOSPlatform::Initialize(const PlatformConfig& config) {
    if (m_initialized) return true;
    m_config = config;
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    m_startTime = std::chrono::duration<double>(duration).count();
    m_initialized = true;
    return true;
}

void IOSPlatform::Shutdown() {
    m_initialized = false;
}

void IOSPlatform::RunMainLoop() {
    // iOS event loop would go here
}

void IOSPlatform::RequestExit() {
    // Request exit
}

Window* IOSPlatform::CreateWindow(int width, int height, const std::string& title) {
    return nullptr;
}

void IOSPlatform::DestroyWindow(Window* window) {
    if (window) {
        window->Destroy();
        delete window;
    }
}

InputManager* IOSPlatform::GetInputManager() {
    return nullptr;
}

FileSystem* IOSPlatform::GetFileSystem() {
    return nullptr;
}

PlatformType IOSPlatform::GetType() const {
    return PlatformType::iOS;
}

std::string IOSPlatform::GetPlatformName() const {
    return "iOS";
}

bool IOSPlatform::IsMobile() const {
    return true;
}

int IOSPlatform::GetScreenWidth() const {
    return 1024;
}

int IOSPlatform::GetScreenHeight() const {
    return 768;
}

float IOSPlatform::GetScreenDensity() const {
    return 2.0f;
}

std::string IOSPlatform::GetResourcePath(const std::string& relativePath) const {
    return relativePath;
}

std::string IOSPlatform::GetWritablePath(const std::string& relativePath) const {
    return relativePath;
}

double IOSPlatform::GetCurrentTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

double IOSPlatform::GetTimeSinceStart() const {
    return GetCurrentTime() - m_startTime;
}

} // namespace SmartRenderer
