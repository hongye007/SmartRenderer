#pragma once

#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Buffer.h"
#include "rendering/VertexArray.h"
#include "rendering/Framebuffer.h"
#include <string>

namespace SmartRenderer {

// Factory functions for creating OpenGL ES resources
Shader* CreateOpenGLESShader(const std::string& vertexSource, const std::string& fragmentSource);
Texture* CreateOpenGLESTexture(int width, int height, int format, const void* data);
Buffer* CreateOpenGLESBuffer(size_t size, const void* data, BufferUsage usage);
VertexArray* CreateOpenGLESVertexArray();
Framebuffer* CreateOpenGLESFramebuffer(int width, int height);

} // namespace SmartRenderer
