#include "rendering/angle/ANGLEConfig.h"

namespace SmartRenderer {

ANGLEConfig::BackendType ANGLEConfigHelper::GetRecommendedBackend() {
    return ANGLEContext::GetDefaultBackend();
}

ANGLEConfig ANGLEConfigHelper::CreateDefault() {
    ANGLEConfig config;
    config.backendType = GetRecommendedBackend();
    return config;
}

ANGLEConfig ANGLEConfigHelper::CreateForBackend(ANGLEConfig::BackendType backend) {
    ANGLEConfig config;
    config.backendType = backend;
    return config;
}

} // namespace SmartRenderer
