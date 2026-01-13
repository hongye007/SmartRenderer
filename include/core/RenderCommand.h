#pragma once

#include "math/MathTypes.h"  // For Color
#include "math/Matrix.h"     // For Matrix4
#include <string>
#include <memory>
#include <vector>

namespace SmartRenderer {

// Forward declarations
class Renderer;
class Shader;
class Texture;
class VertexArray;
class Framebuffer;

// Forward declare ClearFlags (defined in Renderer.h to avoid circular dependency)
enum class ClearFlags;

// Base class for all render commands
class RenderCommand {
public:
    virtual ~RenderCommand() = default;
    virtual void Execute(Renderer& renderer) = 0;
};

// Command: Clear the framebuffer
class ClearCommand : public RenderCommand {
public:
    ClearCommand(ClearFlags flags, const Color& color, float depth = 1.0f, int stencil = 0)
        : m_flags(flags), m_color(color), m_depth(depth), m_stencil(stencil) {}
    
    void Execute(Renderer& renderer) override;

private:
    ClearFlags m_flags;
    Color m_color;
    float m_depth;
    int m_stencil;
};

// Command: Set viewport
class SetViewportCommand : public RenderCommand {
public:
    SetViewportCommand(int x, int y, int width, int height)
        : m_x(x), m_y(y), m_width(width), m_height(height) {}
    
    void Execute(Renderer& renderer) override;

private:
    int m_x, m_y, m_width, m_height;
};

// Command: Bind shader
class BindShaderCommand : public RenderCommand {
public:
    explicit BindShaderCommand(Shader* shader) : m_shader(shader) {}
    
    void Execute(Renderer& renderer) override;

private:
    Shader* m_shader;
};

// Command: Bind texture
class BindTextureCommand : public RenderCommand {
public:
    BindTextureCommand(Texture* texture, int unit = 0) : m_texture(texture), m_unit(unit) {}
    
    void Execute(Renderer& renderer) override;

private:
    Texture* m_texture;
    int m_unit;
};

// Command: Set shader uniform (float)
class SetUniformFloatCommand : public RenderCommand {
public:
    SetUniformFloatCommand(Shader* shader, const std::string& name, float value)
        : m_shader(shader), m_name(name), m_value(value) {}
    
    void Execute(Renderer& renderer) override;

private:
    Shader* m_shader;
    std::string m_name;
    float m_value;
};

// Command: Set shader uniform (int)
class SetUniformIntCommand : public RenderCommand {
public:
    SetUniformIntCommand(Shader* shader, const std::string& name, int value)
        : m_shader(shader), m_name(name), m_value(value) {}
    
    void Execute(Renderer& renderer) override;

private:
    Shader* m_shader;
    std::string m_name;
    int m_value;
};

// Command: Set shader uniform (vec4)
class SetUniformVec4Command : public RenderCommand {
public:
    SetUniformVec4Command(Shader* shader, const std::string& name, float x, float y, float z, float w)
        : m_shader(shader), m_name(name), m_x(x), m_y(y), m_z(z), m_w(w) {}
    
    void Execute(Renderer& renderer) override;

private:
    Shader* m_shader;
    std::string m_name;
    float m_x, m_y, m_z, m_w;
};

// Command: Set shader uniform (matrix4x4)
class SetUniformMatrix4Command : public RenderCommand {
public:
    SetUniformMatrix4Command(Shader* shader, const std::string& name, const Matrix4& matrix)
        : m_shader(shader), m_name(name), m_matrix(matrix) {}
    
    void Execute(Renderer& renderer) override;

private:
    Shader* m_shader;
    std::string m_name;
    Matrix4 m_matrix;
};

// Command: Bind vertex array
class BindVertexArrayCommand : public RenderCommand {
public:
    explicit BindVertexArrayCommand(VertexArray* vao) : m_vao(vao) {}
    
    void Execute(Renderer& renderer) override;

private:
    VertexArray* m_vao;
};

// Command: Bind framebuffer
class BindFramebufferCommand : public RenderCommand {
public:
    explicit BindFramebufferCommand(Framebuffer* framebuffer) : m_framebuffer(framebuffer) {}
    
    void Execute(Renderer& renderer) override;

private:
    Framebuffer* m_framebuffer;
};

// Command: Draw arrays
class DrawArraysCommand : public RenderCommand {
public:
    DrawArraysCommand(int first, int count) : m_first(first), m_count(count) {}
    
    void Execute(Renderer& renderer) override;

private:
    int m_first;
    int m_count;
};

// Command: Draw elements
class DrawElementsCommand : public RenderCommand {
public:
    DrawElementsCommand(int count, int offset = 0) : m_count(count), m_offset(offset) {}
    
    void Execute(Renderer& renderer) override;

private:
    int m_count;
    int m_offset;
};

// Command queue for batch execution
class RenderCommandQueue {
public:
    RenderCommandQueue();
    ~RenderCommandQueue();

    // Add commands to queue
    void Push(std::unique_ptr<RenderCommand> command);
    void PushClear(ClearFlags flags, const Color& color, float depth = 1.0f, int stencil = 0);
    void PushSetViewport(int x, int y, int width, int height);
    void PushBindShader(Shader* shader);
    void PushBindTexture(Texture* texture, int unit = 0);
    void PushSetUniformFloat(Shader* shader, const std::string& name, float value);
    void PushSetUniformInt(Shader* shader, const std::string& name, int value);
    void PushSetUniformVec4(Shader* shader, const std::string& name, float x, float y, float z, float w);
    void PushSetUniformMatrix4(Shader* shader, const std::string& name, const Matrix4& matrix);
    void PushBindVertexArray(VertexArray* vao);
    void PushBindFramebuffer(Framebuffer* framebuffer);
    void PushDrawArrays(int first, int count);
    void PushDrawElements(int count, int offset = 0);

    // Execute all commands in queue
    void Execute(Renderer& renderer);
    
    // Clear all commands
    void Clear();
    
    // Get queue size
    size_t Size() const { return m_commands.size(); }
    bool Empty() const { return m_commands.empty(); }

private:
    std::vector<std::unique_ptr<RenderCommand>> m_commands;
};

} // namespace SmartRenderer
