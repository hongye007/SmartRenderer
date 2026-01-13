#include "platform/Platform.h"
#include <stdexcept>

// Platform implementations - conditionally compile based on target
#if defined(PLATFORM_WINDOWS) || defined(_WIN32)
#include "platform/windows/WindowsPlatform.h"
#endif
#if defined(PLATFORM_MACOS) || (defined(__APPLE__) && !defined(IOS))
#include "platform/macos/MacOSPlatform.h"
#endif
#if defined(PLATFORM_IOS) || defined(IOS)
#include "platform/ios/IOSPlatform.h"
#endif
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include "platform/android/AndroidPlatform.h"
#endif

namespace SmartRenderer {

// Manually specify platform type (no macro checks in switch statement)
std::unique_ptr<Platform> CreatePlatform(PlatformType type) {
    switch (type) {
    case PlatformType::Windows:
#if defined(PLATFORM_WINDOWS) || defined(_WIN32)
        return std::make_unique<WindowsPlatform>();
#else
        throw std::runtime_error("Windows platform not supported in this build");
#endif
    case PlatformType::macOS:
#if defined(PLATFORM_MACOS) || (defined(__APPLE__) && !defined(IOS))
        {
            MacOSPlatform* platform = new MacOSPlatform();
            return std::unique_ptr<Platform>(platform);
        }
#else
        throw std::runtime_error("macOS platform not supported in this build");
#endif
    case PlatformType::iOS:
#if defined(PLATFORM_IOS) || defined(IOS)
        {
            IOSPlatform* platform = new IOSPlatform();
            return std::unique_ptr<Platform>(platform);
        }
#else
        throw std::runtime_error("iOS platform not supported in this build");
#endif
    case PlatformType::Android:
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
        {
            AndroidPlatform* platform = new AndroidPlatform();
            return std::unique_ptr<Platform>(platform);
        }
#else
        throw std::runtime_error("Android platform not supported in this build");
#endif
    case PlatformType::Linux:
        throw std::runtime_error("Linux platform implementation not yet available");
    default:
        throw std::runtime_error("Unknown platform type");
    }
}

// Automatically detect current platform (calls the parameterized version)
std::unique_ptr<Platform> CreatePlatform() {
#if defined(PLATFORM_WINDOWS) || defined(_WIN32)
    return CreatePlatform(PlatformType::Windows);
#elif defined(PLATFORM_MACOS) || (defined(__APPLE__) && !defined(IOS))
    return CreatePlatform(PlatformType::macOS);
#elif defined(PLATFORM_IOS) || defined(IOS)
    return CreatePlatform(PlatformType::iOS);
#elif defined(PLATFORM_ANDROID) || defined(__ANDROID__)
    return CreatePlatform(PlatformType::Android);
#else
    #error "Unknown platform - please define PLATFORM_WINDOWS, PLATFORM_MACOS, PLATFORM_IOS, or PLATFORM_ANDROID"
#endif
}

} // namespace SmartRenderer
