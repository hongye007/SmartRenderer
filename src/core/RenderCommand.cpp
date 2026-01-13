#include "core/RenderCommand.h"
#include "core/Renderer.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/VertexArray.h"
#include "rendering/Framebuffer.h"
#include "math/Matrix.h"

namespace SmartRenderer {

// ClearCommand implementation
void ClearCommand::Execute(Renderer& renderer) {
    renderer.Clear(m_flags, m_color, m_depth, m_stencil);
}

// SetViewportCommand implementation
void SetViewportCommand::Execute(Renderer& renderer) {
    renderer.SetViewport(m_x, m_y, m_width, m_height);
}

// BindShaderCommand implementation
void BindShaderCommand::Execute(Renderer& renderer) {
    renderer.BindShader(m_shader);
}

// BindTextureCommand implementation
void BindTextureCommand::Execute(Renderer& renderer) {
    renderer.BindTexture(m_texture, m_unit);
}

// SetUniformFloatCommand implementation
void SetUniformFloatCommand::Execute(Renderer& renderer) {
    if (m_shader) {
        m_shader->SetUniform(m_name, m_value);
    }
}

// SetUniformIntCommand implementation
void SetUniformIntCommand::Execute(Renderer& renderer) {
    if (m_shader) {
        m_shader->SetUniform(m_name, m_value);
    }
}

// SetUniformVec4Command implementation
void SetUniformVec4Command::Execute(Renderer& renderer) {
    if (m_shader) {
        m_shader->SetUniform(m_name, m_x, m_y, m_z, m_w);
    }
}

// SetUniformMatrix4Command implementation
void SetUniformMatrix4Command::Execute(Renderer& renderer) {
    if (m_shader) {
        m_shader->SetUniform(m_name, m_matrix.m);
    }
}

// BindVertexArrayCommand implementation
void BindVertexArrayCommand::Execute(Renderer& renderer) {
    renderer.BindVertexArray(m_vao);
}

// BindFramebufferCommand implementation
void BindFramebufferCommand::Execute(Renderer& renderer) {
    renderer.BindFramebuffer(m_framebuffer);
}

// DrawArraysCommand implementation
void DrawArraysCommand::Execute(Renderer& renderer) {
    renderer.DrawArrays(m_first, m_count);
}

// DrawElementsCommand implementation
void DrawElementsCommand::Execute(Renderer& renderer) {
    renderer.DrawElements(m_count, m_offset);
}

// RenderCommandQueue implementation
RenderCommandQueue::RenderCommandQueue() {
}

RenderCommandQueue::~RenderCommandQueue() {
    Clear();
}

void RenderCommandQueue::Push(std::unique_ptr<RenderCommand> command) {
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

void RenderCommandQueue::PushClear(ClearFlags flags, const Color& color, float depth, int stencil) {
    Push(std::make_unique<ClearCommand>(flags, color, depth, stencil));
}

void RenderCommandQueue::PushSetViewport(int x, int y, int width, int height) {
    Push(std::make_unique<SetViewportCommand>(x, y, width, height));
}

void RenderCommandQueue::PushBindShader(Shader* shader) {
    Push(std::make_unique<BindShaderCommand>(shader));
}

void RenderCommandQueue::PushBindTexture(Texture* texture, int unit) {
    Push(std::make_unique<BindTextureCommand>(texture, unit));
}

void RenderCommandQueue::PushSetUniformFloat(Shader* shader, const std::string& name, float value) {
    Push(std::make_unique<SetUniformFloatCommand>(shader, name, value));
}

void RenderCommandQueue::PushSetUniformInt(Shader* shader, const std::string& name, int value) {
    Push(std::make_unique<SetUniformIntCommand>(shader, name, value));
}

void RenderCommandQueue::PushSetUniformVec4(Shader* shader, const std::string& name, float x, float y, float z, float w) {
    Push(std::make_unique<SetUniformVec4Command>(shader, name, x, y, z, w));
}

void RenderCommandQueue::PushSetUniformMatrix4(Shader* shader, const std::string& name, const Matrix4& matrix) {
    Push(std::make_unique<SetUniformMatrix4Command>(shader, name, matrix));
}

void RenderCommandQueue::PushBindVertexArray(VertexArray* vao) {
    Push(std::make_unique<BindVertexArrayCommand>(vao));
}

void RenderCommandQueue::PushBindFramebuffer(Framebuffer* framebuffer) {
    Push(std::make_unique<BindFramebufferCommand>(framebuffer));
}

void RenderCommandQueue::PushDrawArrays(int first, int count) {
    Push(std::make_unique<DrawArraysCommand>(first, count));
}

void RenderCommandQueue::PushDrawElements(int count, int offset) {
    Push(std::make_unique<DrawElementsCommand>(count, offset));
}

void RenderCommandQueue::Execute(Renderer& renderer) {
    for (auto& command : m_commands) {
        if (command) {
            command->Execute(renderer);
        }
    }
}

void RenderCommandQueue::Clear() {
    m_commands.clear();
}

} // namespace SmartRenderer
