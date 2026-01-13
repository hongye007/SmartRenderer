# 命令模式解耦实现

## 概述

通过命令模式实现了 `RenderSystem` 和 `Renderer` 的完全解耦。`RenderSystem` 不再直接依赖 `Renderer`，而是生成渲染命令，由外部执行。

## 架构设计

### 核心组件

```
RenderSystem (ECS系统)
    ↓ 生成命令
RenderCommandQueue (命令队列)
    ↓ 执行命令
Renderer (渲染器)
```

### 命令类层次

```
RenderCommand (基类)
├── ClearCommand (清除帧缓冲)
├── SetViewportCommand (设置视口)
├── BindShaderCommand (绑定着色器)
├── BindTextureCommand (绑定纹理)
├── SetUniformFloatCommand (设置浮点uniform)
├── SetUniformIntCommand (设置整数uniform)
├── SetUniformVec4Command (设置vec4 uniform)
├── SetUniformMatrix4Command (设置矩阵uniform)
├── BindVertexArrayCommand (绑定顶点数组)
├── BindFramebufferCommand (绑定帧缓冲)
├── DrawArraysCommand (绘制数组)
└── DrawElementsCommand (绘制索引)
```

## 实现细节

### 1. RenderCommand 基类

```cpp
class RenderCommand {
public:
    virtual ~RenderCommand() = default;
    virtual void Execute(Renderer& renderer) = 0;
};
```

### 2. RenderCommandQueue

管理命令队列，提供便捷的 Push 方法：

```cpp
class RenderCommandQueue {
public:
    void PushClear(ClearFlags flags, const Color& color, ...);
    void PushBindShader(Shader* shader);
    void PushDrawArrays(int first, int count);
    // ... 其他命令
    
    void Execute(Renderer& renderer);  // 执行所有命令
    void Clear();                       // 清空队列
};
```

### 3. RenderSystem 重构

**之前**（直接依赖 Renderer）：
```cpp
class RenderSystem {
    Renderer* m_renderer;  // 直接依赖
    
    void RenderEntity(...) {
        m_renderer->BindShader(...);
        m_renderer->DrawArrays(...);
    }
};
```

**现在**（使用命令模式）：
```cpp
class RenderSystem {
    RenderCommandQueue m_commandQueue;  // 命令队列
    
    void RenderEntity(...) {
        m_commandQueue.PushBindShader(...);
        m_commandQueue.PushDrawArrays(...);
    }
    
    RenderCommandQueue& GetCommandQueue() {
        return m_commandQueue;
    }
};
```

## 使用方式

### 在应用层执行命令

```cpp
// 1. Update ECS World (生成命令)
world->Update(deltaTime);

// 2. 获取 RenderSystem 并执行命令队列
auto* renderSystem = world->GetSystem<RenderSystem>();
if (renderSystem) {
    renderSystem->GetCommandQueue().Execute(*renderer);
}
```

## 优势

### ✅ 完全解耦

- `RenderSystem` 不再依赖 `Renderer`
- `RenderSystem` 可以独立测试
- 不需要 mock `Renderer`

### ✅ 延迟执行

- 命令可以批量生成
- 可以延迟到合适的时机执行
- 支持命令排序和优化

### ✅ 可扩展性

- 可以添加新命令类型
- 可以记录/回放命令
- 可以支持多线程渲染

### ✅ 更好的测试性

- 可以验证命令是否正确生成
- 不需要实际的 Renderer 来测试逻辑

## 命令执行流程

```
1. RenderSystem::Update()
   └── 遍历实体，生成渲染命令
       └── 命令添加到 m_commandQueue

2. 应用层获取命令队列
   └── renderSystem->GetCommandQueue()

3. 执行命令
   └── commandQueue.Execute(*renderer)
       └── 遍历命令，调用 Execute(renderer)
           └── 每个命令调用对应的 Renderer 方法
```

## 未来扩展

### 1. 命令批处理
- 合并相同类型的命令
- 减少状态切换

### 2. 命令记录/回放
- 记录命令序列
- 支持回放调试

### 3. 多线程渲染
- 命令生成和命令执行分离
- 支持多线程命令执行

### 4. 命令验证
- 验证命令参数有效性
- 提前发现错误

## 总结

命令模式成功解耦了 `RenderSystem` 和 `Renderer`：
- ✅ `RenderSystem` 不再持有 `Renderer` 指针
- ✅ 职责清晰：生成命令 vs 执行命令
- ✅ 更好的可测试性和可扩展性
- ✅ 支持延迟执行和批处理
