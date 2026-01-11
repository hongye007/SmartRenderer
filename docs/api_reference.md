# SmartRenderer API Reference

## Core API

### Platform

```cpp
auto platform = SmartRenderer::CreatePlatform(SmartRenderer::PlatformType::Windows);
platform->Initialize(config);
auto window = platform->CreateWindow(800, 600, "My App");
```

### Renderer

```cpp
auto renderer = SmartRenderer::CreateRenderer(window, renderConfig);
renderer->BeginFrame();
renderer->Clear(SmartRenderer::ClearFlags::All);
renderer->EndFrame();
renderer->Present();
```

### Resource Management

```cpp
SmartRenderer::ResourceManager manager;
manager.SetRenderer(renderer.get());
auto shader = manager.LoadShader("basic", "shaders/basic.vert", "shaders/basic.frag");
auto texture = manager.LoadTexture("textures/test.png");
```

### Render Graph

```cpp
auto graph = std::make_unique<SmartRenderer::RenderGraph>();
// Add nodes and connect them
graph->Compile();
graph->Execute(context);
```

## Math Library

### Vectors

```cpp
SmartRenderer::Vector3 v1(1.0f, 2.0f, 3.0f);
SmartRenderer::Vector3 v2(4.0f, 5.0f, 6.0f);
auto result = v1 + v2;
float dot = SmartRenderer::Vector3::Dot(v1, v2);
```

### Matrices

```cpp
auto translation = SmartRenderer::Matrix4::CreateTranslation(SmartRenderer::Vector3(1, 2, 3));
auto rotation = SmartRenderer::Matrix4::CreateRotationX(1.57f);
auto projection = SmartRenderer::Matrix4::CreatePerspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
```

## For more examples, see the samples/ directory
