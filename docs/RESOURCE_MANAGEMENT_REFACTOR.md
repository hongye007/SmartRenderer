# 资源管理重构总结

## 重构目标

以最优的方式重构资源管理，解决以下问题：
1. ResourceRegistry 持有原始指针，不够安全
2. ResourceHandle 删除器捕获原始指针
3. 资源管理路径不统一（三种方式）
4. ECS 组件资源管理不一致

## 重构方案

### 1. ResourceFactory - 统一资源管理接口 ✅

**设计**：
- 组合 `ResourceManager`（数据加载）和 `ResourceRegistry`（GPU资源管理）
- 提供单一、安全、便捷的 API
- 自动管理资源生命周期

**优势**：
- ✅ 统一接口：一个类解决所有资源管理需求
- ✅ 自动生命周期管理：返回 ResourceHandle
- ✅ 资源去重：自动检查是否已加载
- ✅ 简化使用：一步完成加载和注册

**使用示例**：
```cpp
// 创建 ResourceFactory
ResourceFactory factory(renderer.get());

// 加载纹理（一步完成）
TextureHandle texture = factory.LoadTexture("texture.jpg", "main_texture");

// 创建着色器（一步完成）
ShaderHandle shader = factory.CreateShader("basic", vertexSource, fragmentSource);

// 获取资源
TextureHandle tex = factory.GetTexture("main_texture");

// 自动清理（factory 销毁时）
```

### 2. 改进 ResourceHandle - 支持 weak_ptr ✅

**设计**：
- 保留原始指针版本（向后兼容）
- 新增 weak_ptr 版本（更安全）
- 删除器安全检查

**优势**：
- ✅ 向后兼容：现有代码无需修改
- ✅ 更安全：支持 weak_ptr 检查
- ✅ 异常安全：try-catch 保护

**代码**：
```cpp
// 原始指针版本（向后兼容）
ResourceHandle(ResourceType* resource, Renderer* renderer, 
               std::function<void(Renderer*, ResourceType*)> destroyFunc);

// weak_ptr 版本（更安全，推荐新代码使用）
ResourceHandle(ResourceType* resource, 
               std::weak_ptr<Renderer> renderer,
               std::function<void(Renderer*, ResourceType*)> destroyFunc);
```

### 3. 更新 ECS Material 组件 - 使用 ResourceHandle ✅

**设计**：
- `shader` 从 `Shader*` 改为 `ShaderHandle`
- `albedoTexture` 等从 `shared_ptr<Texture>` 改为 `TextureHandle`
- 统一使用 ResourceHandle

**优势**：
- ✅ 统一管理：所有资源使用相同的句柄类型
- ✅ 自动清理：资源生命周期自动管理
- ✅ 类型安全：编译时检查

**代码**：
```cpp
class Material : public Component {
public:
    ShaderHandle shader;              // 统一使用 ResourceHandle
    TextureHandle albedoTexture;      // 统一使用 ResourceHandle
    TextureHandle normalTexture;
    // ...
    
    bool IsValid() const { return shader.IsValid(); }
};
```

### 4. 更新 RenderSystem - 适配 ResourceHandle ✅

**设计**：
- 使用 `handle.Get()` 获取原始指针
- 检查 `handle.IsValid()` 验证有效性

**代码**：
```cpp
if (!material->shader.IsValid()) {
    return;
}

Shader* shader = material->shader.Get();
m_commandQueue.PushBindShader(shader);
```

## 架构对比

### 重构前

```
用户代码
  ├── ResourceManager (数据加载)
  ├── Renderer (创建GPU资源)
  └── ResourceRegistry (管理GPU资源)
       └── ResourceHandle (自动清理)

问题：
- 三种资源管理路径
- 需要手动协调多个模块
- ECS 组件使用原始指针
```

### 重构后

```
用户代码
  └── ResourceFactory (统一接口)
       ├── ResourceManager (内部，数据加载)
       └── ResourceRegistry (内部，GPU资源管理)
            └── ResourceHandle (自动清理)

优势：
- 单一统一接口
- 自动管理生命周期
- ECS 组件使用 ResourceHandle
```

## 使用指南

### 方式 1：使用 ResourceFactory（推荐）⭐

```cpp
// 创建工厂
ResourceFactory factory(renderer.get());

// 加载资源（一步完成）
TextureHandle texture = factory.LoadTexture("texture.jpg");
ShaderHandle shader = factory.LoadShader("basic", "vert.glsl", "frag.glsl");

// 使用资源
renderer->BindTexture(texture.Get(), 0);
renderer->BindShader(shader.Get());

// 自动清理（factory 销毁时）
```

### 方式 2：直接使用 ResourceRegistry（高级用法）

```cpp
// 创建注册表
ResourceRegistry registry(renderer.get());

// 创建并注册资源
Texture* texture = renderer->CreateTexture(...);
TextureHandle handle = registry.RegisterTexture("name", texture);

// 使用句柄
renderer->BindTexture(handle.Get(), 0);
```

### 方式 3：使用 ResourceHandle（手动管理）

```cpp
// 创建资源
Texture* texture = renderer->CreateTexture(...);

// 创建句柄
auto deleter = [](Renderer* r, Texture* t) { r->DestroyTexture(t); };
TextureHandle handle(texture, renderer.get(), deleter);

// 使用句柄
renderer->BindTexture(handle.Get(), 0);
```

## 迁移指南

### 从旧代码迁移

1. **替换 ResourceManager + ResourceRegistry 组合**：
   ```cpp
   // 旧代码
   ResourceManager dataMgr;
   ResourceRegistry registry(renderer.get());
   TextureData data;
   dataMgr.LoadTextureData("tex.jpg", data);
   Texture* tex = renderer->CreateTexture(...);
   TextureHandle handle = registry.RegisterTexture("tex", tex);
   
   // 新代码
   ResourceFactory factory(renderer.get());
   TextureHandle handle = factory.LoadTexture("tex.jpg", "tex");
   ```

2. **更新 Material 组件**：
   ```cpp
   // 旧代码
   Material material;
   material.shader = shaderPtr;  // Shader*
   material.albedoTexture = std::make_shared<Texture>(...);
   
   // 新代码
   Material material;
   material.shader = shaderHandle;  // ShaderHandle
   material.albedoTexture = textureHandle;  // TextureHandle
   ```

3. **更新资源访问**：
   ```cpp
   // 旧代码
   if (material.shader) {
       renderer->BindShader(material.shader);
   }
   
   // 新代码
   if (material.shader.IsValid()) {
       renderer->BindShader(material.shader.Get());
   }
   ```

## 性能考虑

- **ResourceHandle**：使用 `shared_ptr`，引用计数开销很小
- **ResourceFactory**：内部使用 `unordered_map`，O(1) 查找
- **资源去重**：自动检查，避免重复加载

## 安全性改进

1. ✅ **ResourceHandle 删除器**：使用 try-catch 保护
2. ✅ **支持 weak_ptr**：可以安全检查 renderer 有效性
3. ✅ **自动清理**：资源在最后一个句柄销毁时自动清理
4. ✅ **类型安全**：编译时类型检查

## 总结

### 重构成果

1. ✅ **ResourceFactory**：统一资源管理接口
2. ✅ **ResourceHandle**：支持 weak_ptr，更安全
3. ✅ **ECS Material**：统一使用 ResourceHandle
4. ✅ **RenderSystem**：适配 ResourceHandle

### 优势

- ✅ **统一接口**：ResourceFactory 提供单一入口
- ✅ **自动管理**：资源生命周期自动管理
- ✅ **类型安全**：编译时检查
- ✅ **向后兼容**：保留原始指针支持
- ✅ **易于使用**：一步完成加载和注册

### 下一步

- [ ] 添加资源热重载支持
- [ ] 添加资源去重机制（基于内容哈希）
- [ ] 统一错误处理（使用 std::expected）
- [ ] 添加资源统计和监控

## 示例代码

完整示例请参考：
- `samples/resource_factory_demo/main.cpp` - ResourceFactory 使用示例
- `samples/resource_handle_demo/main.cpp` - ResourceHandle 使用示例
