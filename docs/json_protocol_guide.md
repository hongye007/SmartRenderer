# JSON 渲染协议指南

## 概述

SmartRenderer 支持通过 JSON 配置文件定义渲染图（Render Graph），实现灵活的渲染管线配置。

## JSON 协议格式

### 基本结构

```json
{
  "renderGraph": {
    "name": "GraphName",
    "nodes": [
      {
        "type": "NodeType",
        "name": "nodeName",
        "shader": {
          "vertex": "#version 300 es\n...",
          "fragment": "#version 300 es\n..."
        },
        "uniforms": {
          "uColor": [1.0, 0.0, 0.0]
        },
        "inputs": ["input1", "input2"],
        "outputs": ["output1", "output2"]
      }
    ],
    "connections": [
      {
        "from": "node1",
        "output": "output1",
        "to": "node2",
        "input": "input1"
      }
    ],
    "output": {
      "node": "outputNodeName",
      "resource": "outputResourceName"
    }
  }
}
```

### 字段说明

- **renderGraph.name**: 渲染图的名称（可选）
- **renderGraph.nodes**: 节点数组
  - **type**: 节点类型（如 "ClearPass", "GeometryPass", "ColorPass", "TriangleGeometry" 等）
  - **name**: 节点名称（唯一标识符）
  - **shader**: Shader 配置（可选）
    - **vertex**: 顶点着色器源代码
    - **fragment**: 片段着色器源代码
  - **uniforms**: Uniform 变量配置（可选）
    - 键为 uniform 变量名，值为浮点数数组
  - **inputs**: 输入资源列表（可选）
  - **outputs**: 输出资源列表
- **renderGraph.connections**: 节点连接数组（显式定义图的组装逻辑）
  - **from**: 源节点名称
  - **output**: 源节点的输出资源
  - **to**: 目标节点名称
  - **input**: 目标节点的输入资源
- **renderGraph.output**: 最终输出配置
  - **node**: 输出节点名称
  - **resource**: 输出资源名称

## 示例配置

### 1. 前向渲染（Forward Rendering）

```json
{
  "renderGraph": {
    "name": "ForwardRendering",
    "nodes": [
      {
        "type": "GeometryPass",
        "name": "geometryPass",
        "outputs": ["colorBuffer", "depthBuffer"]
      }
    ],
    "output": {
      "node": "geometryPass",
      "resource": "colorBuffer"
    }
  }
}
```

### 2. 三角形分色渲染（Triangle Split View）

```json
{
  "renderGraph": {
    "name": "TriangleSplitRendering",
    "nodes": [
      {
        "type": "TriangleGeometry",
        "name": "triangleGeometry",
        "shader": {
          "vertex": "#version 300 es\nprecision mediump float;\nlayout(location = 0) in vec2 aPosition;\nvoid main() { gl_Position = vec4(aPosition, 0.0, 1.0); }",
          "fragment": "#version 300 es\nprecision mediump float;\nuniform vec3 uColor;\nout vec4 FragColor;\nvoid main() { FragColor = vec4(uColor, 1.0); }"
        },
        "outputs": ["triangleMesh"]
      },
      {
        "type": "ColorPass",
        "name": "blueColorNode",
        "shader": {
          "vertex": "#version 300 es\nprecision mediump float;\nlayout(location = 0) in vec2 aPosition;\nvoid main() { gl_Position = vec4(aPosition, 0.0, 1.0); }",
          "fragment": "#version 300 es\nprecision mediump float;\nuniform vec3 uColor;\nout vec4 FragColor;\nvoid main() { FragColor = vec4(0.0, 0.0, 1.0, 1.0); }"
        },
        "inputs": ["triangleMesh"],
        "outputs": ["blueTriangle"],
        "uniforms": {
          "uColor": [0.0, 0.0, 1.0]
        }
      },
      {
        "type": "ColorPass",
        "name": "redColorNode",
        "shader": {
          "vertex": "#version 300 es\nprecision mediump float;\nlayout(location = 0) in vec2 aPosition;\nvoid main() { gl_Position = vec4(aPosition, 0.0, 1.0); }",
          "fragment": "#version 300 es\nprecision mediump float;\nuniform vec3 uColor;\nout vec4 FragColor;\nvoid main() { FragColor = vec4(1.0, 0.0, 0.0, 1.0); }"
        },
        "inputs": ["triangleMesh"],
        "outputs": ["redTriangle"],
        "uniforms": {
          "uColor": [1.0, 0.0, 0.0]
        }
      },
      {
        "type": "Composite",
        "name": "compositeNode",
        "inputs": ["blueTriangle", "redTriangle"],
        "outputs": ["finalOutput"]
      }
    ],
    "connections": [
      {
        "from": "triangleGeometry",
        "output": "triangleMesh",
        "to": "blueColorNode",
        "input": "triangleMesh"
      },
      {
        "from": "triangleGeometry",
        "output": "triangleMesh",
        "to": "redColorNode",
        "input": "triangleMesh"
      },
      {
        "from": "blueColorNode",
        "output": "blueTriangle",
        "to": "compositeNode",
        "input": "blueTriangle"
      },
      {
        "from": "redColorNode",
        "output": "redTriangle",
        "to": "compositeNode",
        "input": "redTriangle"
      }
    ],
    "output": {
      "node": "compositeNode",
      "resource": "finalOutput"
    }
  }
}
```

## 支持的节点类型

### ClearPass / Clear
清屏节点，清除颜色和深度缓冲区。

**输入**: 无  
**输出**: 无

### GeometryPass / Geometry
几何渲染节点，渲染 3D 几何体。

**输入**: 无  
**输出**: colorBuffer, depthBuffer

### PostProcess
后处理节点，应用后处理效果。

**输入**: colorBuffer  
**输出**: finalColor

### TriangleGeometry
三角形几何节点，准备三角形网格数据。支持 shader 配置。

**输入**: 无  
**输出**: triangleMesh  
**Shader**: 顶点和片段着色器（可选）

### ColorPass
颜色处理节点，将几何体渲染为指定颜色。支持 shader 和 uniform 配置。

**输入**: triangleMesh  
**输出**: coloredTriangle  
**Shader**: 顶点和片段着色器  
**Uniforms**: uColor (vec3) - 颜色值

### Composite
合成节点，将多个渲染结果组合显示。

**输入**: blueTriangle, redTriangle  
**输出**: finalOutput

## 使用方式

### 1. 创建 JSON 配置文件

在 `configs/` 目录下创建 JSON 文件，例如 `my_render_graph.json`。

### 2. 在代码中加载

```cpp
#include "core/RenderProtocol.h"

RenderProtocol protocol;
if (protocol.LoadFromFile("configs/my_render_graph.json")) {
    std::cout << "Protocol loaded successfully" << std::endl;
    
    RenderContext context(renderer.get());
    
    // 在渲染循环中执行
    while (!window->ShouldClose()) {
        renderer->BeginFrame();
        protocol.Execute(context);
        renderer->EndFrame();
        renderer->Present();
    }
}
```

### 3. 程序化构建（备用方案）

如果 JSON 加载失败，可以程序化构建渲染图：

```cpp
auto renderGraph = std::make_unique<RenderGraph>();

// 添加节点
auto node1 = renderGraph->AddNode(std::make_unique<MyNode>());
auto node2 = renderGraph->AddNode(std::make_unique<MyOtherNode>());

// 连接节点
renderGraph->Connect("node1", "output", "node2", "input");

// 设置输出
renderGraph->SetOutput("node2", "finalOutput");

// 编译
renderGraph->Compile();
```

## JSON 解析实现

当前实现使用简单的正则表达式解析 JSON，支持：
- 字符串提取
- 字符串数组提取
- 节点数组解析
- 输出配置解析

### 解析流程

1. **加载文件**: 读取 JSON 文件内容
2. **提取节点**: 解析所有节点定义，包括 shader 和 uniform 配置
3. **创建节点**: 根据类型创建节点实例，从 JSON 中提取 shader 并创建着色器
4. **建立连接**: 解析 `connections` 数组，显式建立节点连接（图的组装逻辑）
5. **设置输出**: 配置最终输出节点
6. **编译图**: 编译渲染图，确定执行顺序

### Shader 配置

每个节点可以包含 `shader` 配置，指定渲染方式：

- **vertex**: 顶点着色器源代码（GLSL ES 3.0）
- **fragment**: 片段着色器源代码（GLSL ES 3.0）

着色器代码支持转义的换行符（`\n`），可以写多行代码。

### Uniform 配置

节点可以包含 `uniforms` 配置，指定着色器 uniform 变量的值：

```json
"uniforms": {
  "uColor": [1.0, 0.0, 0.0],  // vec3
  "uTime": 0.5                 // float (单元素数组)
}
```

### 连接配置

`connections` 数组显式定义了图的组装逻辑，每个连接指定：
- 源节点和输出资源
- 目标节点和输入资源

这允许完全控制渲染图的拓扑结构。

## 扩展节点类型

要添加新的节点类型，需要：

1. **实现节点类**: 继承 `RenderNode` 并实现接口
2. **注册到工厂**: 在 `RenderProtocol::CreateNodeFromType` 中添加类型映射
3. **更新 JSON**: 在 JSON 配置中使用新类型

示例：

```cpp
// 在 CreateNodeFromType 中添加
if (type == "MyCustomNode") {
    class MyCustomNodeImpl : public RenderNode {
        // 实现接口
    };
    return std::make_unique<MyCustomNodeImpl>(params);
}
```

## 配置文件位置

示例程序会尝试以下路径查找配置文件：

1. `configs/triangle_split.json`
2. `../configs/triangle_split.json`
3. `../../configs/triangle_split.json`
4. `../../../configs/triangle_split.json`
5. `configs/forward_rendering.json`
6. `../configs/forward_rendering.json`

## 注意事项

1. **节点名称必须唯一**: 每个节点必须有唯一的名称
2. **输入/输出匹配**: 节点的输入必须与上游节点的输出匹配
3. **输出节点**: 必须指定一个输出节点作为最终输出
4. **编译顺序**: 渲染图会根据依赖关系自动确定执行顺序

## 未来改进

- [ ] 使用完整的 JSON 库（如 nlohmann/json）进行更可靠的解析
- [ ] 支持节点参数配置
- [ ] 支持条件节点和分支
- [ ] 支持循环和迭代节点
- [ ] 可视化渲染图编辑器
