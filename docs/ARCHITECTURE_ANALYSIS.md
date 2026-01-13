# SmartRenderer 架构分析

## 当前模块结构

### 1. 核心模块层次

```
Application (samples/)
    ↓
Engine API Layer
    ├── SmartRenderer.h (统一入口)
    ├── Platform (平台抽象)
    ├── Window (窗口管理)
    └── Renderer (渲染器抽象)
        ↓
Core Systems
    ├── ResourceManager (资源管理)
    ├── RenderGraph (渲染图系统)
    ├── RenderProtocol (渲染协议/JSON配置)
    └── ECS (实体组件系统)
        ↓
Rendering Backend
    ├── OpenGLESRenderer (OpenGL ES实现)
    ├── ANGLEContext (ANGLE上下文)
    └── OpenGLESResourceFactory (资源工厂)
        ↓
Platform Implementation
    ├── MacOSWindow (macOS窗口)
    ├── WindowsWindow (Windows窗口)
    └── AndroidPlatform (Android平台)
```

## 模块依赖关系

### 依赖图

```
Platform
  └── Window (平台创建窗口)
       └── Renderer (渲染器需要窗口)
            ├── ResourceManager (需要Renderer创建资源)
            ├── RenderSystem (ECS系统需要Renderer)
            ├── RenderGraph (需要Renderer执行)
            └── ANGLEContext (渲染器需要上下文)
```

## 发现的问题

### 🔴 严重问题

#### 1. **ResourceManager 的生命周期管理混乱**
- **问题**：ResourceManager 持有 Renderer 的原始指针，但没有所有权
- **风险**：如果 Renderer 先于 ResourceManager 销毁，会导致悬空指针
- **位置**：`ResourceManager::m_renderer` 是原始指针
- **建议**：使用弱引用或确保清理顺序

#### 2. **资源所有权不明确**
- **问题**：ResourceManager 管理资源，但资源由 Renderer 创建
- **风险**：ResourceManager::Clear() 调用时，Renderer 可能已销毁
- **位置**：`ResourceManager::Clear()` 需要 Renderer 来销毁资源
- **建议**：在 ResourceManager 析构时检查 Renderer 是否有效

#### 3. **双重资源管理**
- **问题**：用户可以直接通过 Renderer 创建资源，也可以通过 ResourceManager
- **风险**：资源可能被重复管理或泄漏
- **位置**：`texture_sample` 中既有 ResourceManager 加载，也有直接创建
- **建议**：统一资源创建接口，或明确区分用途

### 🟡 设计问题

#### 4. **RenderSystem 与 Renderer 紧耦合**
- **问题**：RenderSystem 直接依赖 Renderer，而不是抽象接口
- **影响**：ECS 系统与渲染实现耦合
- **位置**：`RenderSystem` 持有 `Renderer*`
- **建议**：通过 RenderContext 或命令队列解耦

#### 5. **RenderGraph 与 Renderer 的关系不清晰**
- **问题**：RenderGraph 需要 Renderer，但依赖关系不明确
- **影响**：执行顺序和资源管理可能混乱
- **位置**：`RenderGraph::Execute(RenderContext&)` 的实现
- **建议**：明确 RenderGraph 作为渲染管线的编排器

#### 6. **缺少统一的资源清理机制**
- **问题**：每个模块自己管理资源清理，容易遗漏
- **影响**：资源泄漏、崩溃风险
- **位置**：各模块的析构函数
- **建议**：引入 RAII 包装器或统一的资源管理器

### 🟢 改进建议

#### 7. **模块职责划分**
- **ResourceManager**：应该只负责加载和缓存，不应该持有 Renderer
- **Renderer**：应该提供资源创建接口，但不管理生命周期
- **建议**：引入 ResourceHandle 或智能指针管理资源生命周期

#### 8. **依赖注入**
- **问题**：ResourceManager 需要手动设置 Renderer
- **建议**：通过构造函数注入或工厂模式

#### 9. **错误处理**
- **问题**：资源创建失败时，错误处理不一致
- **建议**：统一错误处理机制（异常或错误码）

## 推荐的架构改进

### 方案 1：资源句柄模式

```cpp
// 资源句柄，自动管理生命周期
class TextureHandle {
    std::shared_ptr<Texture> m_texture;
    Renderer* m_renderer;
public:
    ~TextureHandle() {
        if (m_renderer && m_texture) {
            m_renderer->DestroyTexture(m_texture.get());
        }
    }
};

// ResourceManager 返回句柄
TextureHandle ResourceManager::LoadTexture(const std::string& path);
```

### 方案 2：资源注册表

```cpp
// 统一的资源注册表
class ResourceRegistry {
    std::map<std::string, std::shared_ptr<Resource>> m_resources;
    Renderer* m_renderer;
public:
    void RegisterResource(const std::string& name, Resource* res);
    void UnregisterResource(const std::string& name);
    void Clear(); // 自动清理所有资源
};
```

### 方案 3：命令模式解耦

```cpp
// RenderSystem 不直接调用 Renderer
class RenderCommand {
    virtual void Execute(Renderer& renderer) = 0;
};

// RenderSystem 生成命令
class RenderSystem {
    void Update(World& world) {
        // 生成渲染命令
        m_commandQueue.push(new DrawEntityCommand(...));
    }
};
```

## 当前架构的优点

✅ **清晰的层次结构**：平台 → 渲染器 → 应用
✅ **良好的抽象**：Renderer 接口抽象了底层实现
✅ **跨平台支持**：通过 ANGLE 实现跨平台
✅ **模块化设计**：各模块职责相对清晰

## 总结

当前架构整体合理，但存在以下主要问题：
1. **资源生命周期管理**需要改进
2. **模块间依赖**需要更明确的契约
3. **错误处理**需要统一机制

建议优先解决资源管理问题，这是最可能导致崩溃的地方。
