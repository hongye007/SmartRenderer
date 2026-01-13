# 资源句柄和资源注册表使用指南

## 概述

资源句柄（ResourceHandle）和资源注册表（ResourceRegistry）提供了自动化的资源生命周期管理，解决了资源泄漏和清理顺序的问题。

## 核心组件

### 1. ResourceHandle（资源句柄）

自动管理资源生命周期的智能指针包装器。

**特性**：
- ✅ 自动清理：资源在最后一个句柄销毁时自动销毁
- ✅ 引用计数：使用 `shared_ptr` 实现引用计数
- ✅ 类型安全：模板化设计，编译时类型检查
- ✅ 零开销：与直接使用指针性能相同

### 2. ResourceRegistry（资源注册表）

统一管理所有 GPU 资源的注册表。

**特性**：
- ✅ 集中管理：所有资源在一个地方管理
- ✅ 自动清理：注册表销毁时自动清理所有资源
- ✅ 资源查找：通过名称快速查找资源
- ✅ 引用计数：多个句柄可以共享同一资源

## 使用方式

### 方式 1：使用 ResourceHandle（推荐）

```cpp
// 创建资源并获取句柄
Renderer* renderer = ...;
Shader* shader = renderer->CreateShader(vertexSource, fragmentSource);

// 创建句柄（自动管理生命周期）
auto deleter = [](Renderer* r, Shader* s) { r->DestroyShader(s); };
ShaderHandle shaderHandle(shader, renderer, deleter);

// 使用句柄（就像使用普通指针）
if (shaderHandle) {
    renderer->BindShader(shaderHandle.Get());
    // 或者
    shaderHandle->Bind();
}

// 句柄销毁时，资源自动清理
// 不需要手动调用 DestroyShader
```

### 方式 2：使用 ResourceRegistry（推荐用于复杂项目）

```cpp
// 创建注册表
ResourceRegistry registry(renderer.get());

// 创建并注册资源
Shader* shader = renderer->CreateShader(vertexSource, fragmentSource);
ShaderHandle shaderHandle = registry.RegisterShader("my_shader", shader);

// 或者一步完成
Texture* texture = renderer->CreateTexture(width, height, format, data);
TextureHandle textureHandle = registry.RegisterTexture("my_texture", texture);

// 稍后获取资源
ShaderHandle shader2 = registry.GetShader("my_shader");
if (shader2) {
    // 使用资源
    renderer->BindShader(shader2.Get());
}

// 注册表销毁时，所有资源自动清理
```

### 方式 3：结合 ResourceManager 使用

```cpp
// ResourceManager 加载数据
ResourceManager resourceManager;
TextureData textureData;
resourceManager.LoadTextureData("texture.jpg", textureData);

// 通过 Renderer 创建资源
Texture* texture = renderer->CreateTexture(
    textureData.width,
    textureData.height,
    textureData.format,
    textureData.data.data()
);

// 注册到注册表（自动管理生命周期）
ResourceRegistry registry(renderer.get());
TextureHandle textureHandle = registry.RegisterTexture("texture.jpg", texture);

// 使用句柄
renderer->BindTexture(textureHandle.Get(), 0);

// 不需要手动清理，注册表会处理
```

## 完整示例

```cpp
#include "resource/ResourceRegistry.h"
#include "resource/ResourceManager.h"
#include "resource/ResourceHelper.h"

// 初始化
ResourceRegistry registry(renderer.get());
ResourceManager resourceManager;

// 加载并创建纹理
TextureData textureData;
if (resourceManager.LoadTextureData("texture.jpg", textureData)) {
    Texture* texture = renderer->CreateTexture(
        textureData.width, textureData.height,
        textureData.format, textureData.data.data()
    );
    
    // 注册到注册表
    TextureHandle textureHandle = registry.RegisterTexture("texture.jpg", texture);
    
    // 使用句柄
    renderer->BindTexture(textureHandle.Get(), 0);
}

// 创建着色器
ShaderSource shaderSource;
if (resourceManager.LoadShaderSource("basic", "vertex.glsl", "fragment.glsl", shaderSource)) {
    Shader* shader = ResourceHelper::CreateShaderFromCache(resourceManager, "basic", renderer.get());
    
    // 注册到注册表
    ShaderHandle shaderHandle = registry.RegisterShader("basic", shader);
    
    // 使用句柄
    renderer->BindShader(shaderHandle.Get());
}

// 清理：注册表销毁时自动清理所有资源
// 不需要手动调用 DestroyTexture/DestroyShader
```

## 优势对比

### 之前（手动管理）

```cpp
// 创建资源
Texture* texture = renderer->CreateTexture(...);
Shader* shader = renderer->CreateShader(...);

// 使用资源
renderer->BindTexture(texture, 0);
renderer->BindShader(shader);

// 必须手动清理（容易忘记或顺序错误）
renderer->DestroyTexture(texture);
renderer->DestroyShader(shader);
```

**问题**：
- ❌ 容易忘记清理
- ❌ 清理顺序错误会导致崩溃
- ❌ 资源泄漏风险

### 现在（自动管理）

```cpp
// 创建并注册资源
ResourceRegistry registry(renderer.get());
TextureHandle texture = registry.RegisterTexture("tex", renderer->CreateTexture(...));
ShaderHandle shader = registry.RegisterShader("shader", renderer->CreateShader(...));

// 使用资源
renderer->BindTexture(texture.Get(), 0);
renderer->BindShader(shader.Get());

// 自动清理（注册表销毁时）
// 或者资源句柄销毁时（如果不在注册表中）
```

**优势**：
- ✅ 自动清理，不会泄漏
- ✅ 正确的清理顺序
- ✅ 引用计数，可以安全共享
- ✅ 类型安全

## 最佳实践

### 1. 使用 ResourceRegistry 管理所有资源

```cpp
// 在应用初始化时创建注册表
ResourceRegistry resourceRegistry(renderer.get());

// 所有资源都通过注册表管理
TextureHandle texture = resourceRegistry.RegisterTexture("name", texturePtr);
ShaderHandle shader = resourceRegistry.RegisterShader("name", shaderPtr);
```

### 2. 使用 ResourceHandle 传递资源

```cpp
// 函数参数使用句柄
void RenderEntity(TextureHandle texture, ShaderHandle shader) {
    renderer->BindTexture(texture.Get(), 0);
    renderer->BindShader(shader.Get());
    // 资源生命周期自动管理
}

// 调用
RenderEntity(textureHandle, shaderHandle);
```

### 3. 结合 ResourceManager 使用

```cpp
// ResourceManager: 加载和缓存数据
ResourceManager resourceManager;
TextureData data;
resourceManager.LoadTextureData("texture.jpg", data);

// ResourceRegistry: 管理 GPU 资源生命周期
ResourceRegistry registry(renderer.get());
Texture* texture = renderer->CreateTexture(...);
TextureHandle handle = registry.RegisterTexture("texture.jpg", texture);
```

## 注意事项

1. **Renderer 生命周期**：ResourceRegistry 持有 Renderer 指针，确保 Renderer 在注册表之后销毁
2. **资源共享**：多个句柄可以共享同一资源，资源在最后一个句柄销毁时清理
3. **线程安全**：当前实现不是线程安全的，如需多线程使用需要加锁

## 总结

资源句柄和资源注册表提供了：
- ✅ 自动化的资源生命周期管理
- ✅ 防止资源泄漏
- ✅ 正确的清理顺序
- ✅ 更好的代码可维护性

建议在项目中使用 ResourceRegistry 统一管理所有 GPU 资源。
