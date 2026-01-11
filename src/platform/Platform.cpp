#include "platform/Platform.h"
#include "platform/windows/WindowsPlatform.h"
#include <stdexcept>

// Platform implementations - conditionally compile based on target
#if defined(PLATFORM_MACOS) || defined(__APPLE__) && !defined(IOS)
#include "platform/macos/MacOSPlatform.h"
#endif
#if defined(PLATFORM_IOS) || defined(IOS)
#include "platform/ios/IOSPlatform.h"
#endif
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include "platform/android/AndroidPlatform.h"
#endif

namespace SmartRenderer {

std::unique_ptr<Platform> CreatePlatform(PlatformType type) {
    switch (type) {
    case PlatformType::Windows:
        return std::make_unique<WindowsPlatform>();
#if defined(PLATFORM_MACOS) || (defined(__APPLE__) && !defined(IOS))
    case PlatformType::macOS: {
        MacOSPlatform* platform = new MacOSPlatform();
        return std::unique_ptr<Platform>(platform);
    }
#endif
#if defined(PLATFORM_IOS) || defined(IOS)
    case PlatformType::iOS: {
        IOSPlatform* platform = new IOSPlatform();
        return std::unique_ptr<Platform>(platform);
    }
#endif
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
    case PlatformType::Android: {
        AndroidPlatform* platform = new AndroidPlatform();
        return std::unique_ptr<Platform>(platform);
    }
#endif
    case PlatformType::Linux:
        throw std::runtime_error("Linux platform implementation not yet available");
    default:
        throw std::runtime_error("Unknown platform type");
    }
}

} // namespace SmartRenderer
