# 描边算法说明

## 算法概述

当前实现的描边算法是一个**基于边缘检测的描边算法**，在 Fragment Shader 中实时计算每个像素是否应该绘制描边。

## 算法原理

### 1. 基本思路

描边的核心思想是：**检测 mask 图的边缘**，在边缘处绘制描边颜色。

边缘的定义：**当前像素在 mask 内，但周围有像素在 mask 外**。

### 2. 算法步骤

#### 步骤 1：采样当前像素的 mask 值
```glsl
float maskValue = texture2D(uMaskTexture, vTexCoord).r;
bool insideMask = maskValue > threshold; // threshold = 0.5
```

#### 步骤 2：计算周围 8 个相邻像素的位置
```glsl
vec2 offsets[8];
offsets[0] = vec2(-uOutlineWidth, -uOutlineWidth) * texelSize; // 左上
offsets[1] = vec2( 0.0, -uOutlineWidth) * texelSize;            // 上
offsets[2] = vec2( uOutlineWidth, -uOutlineWidth) * texelSize;   // 右上
offsets[3] = vec2(-uOutlineWidth,  0.0) * texelSize;            // 左
offsets[4] = vec2( uOutlineWidth,  0.0) * texelSize;            // 右
offsets[5] = vec2(-uOutlineWidth,  uOutlineWidth) * texelSize;  // 左下
offsets[6] = vec2( 0.0,  uOutlineWidth) * texelSize;            // 下
offsets[7] = vec2( uOutlineWidth,  uOutlineWidth) * texelSize;  // 右下
```

**8 邻域示意图**：
```
[0] [1] [2]
[3] [X] [4]  <- X 是当前像素
[5] [6] [7]
```

#### 步骤 3：检查周围像素
```glsl
for (int i = 0; i < 8; i++) {
    float neighborMask = texture2D(uMaskTexture, vTexCoord + offsets[i]).r;
    bool neighborInside = neighborMask > threshold;
    
    // 如果当前像素在 mask 内，但邻居在 mask 外，则绘制描边
    if (insideMask && !neighborInside) {
        outline = 1.0;
        break;
    }
}
```

#### 步骤 4：混合颜色
```glsl
vec3 finalColor = mix(originalColor.rgb, outlineColor, outline);
```

## 算法特点

### 优点
1. **实时计算**：在 GPU 上并行执行，性能好
2. **简单直观**：逻辑清晰，易于理解
3. **可调参数**：
   - `uOutlineWidth`：控制描边宽度（像素）
   - `uOutlineColor`：描边颜色
   - `threshold`：mask 阈值（0.5）

### 局限性
1. **只检测 8 邻域**：只能检测直接相邻的像素，描边宽度受限于采样距离
2. **硬边缘**：描边是二值的（0 或 1），没有平滑过渡
3. **阈值固定**：mask 阈值硬编码为 0.5

## 算法流程图

```
对于每个像素：
  ├─ 采样当前像素的 mask 值
  ├─ 判断当前像素是否在 mask 内 (maskValue > 0.5)
  │
  ├─ 如果当前像素在 mask 内：
  │   ├─ 检查周围 8 个相邻像素
  │   ├─ 如果任何一个邻居在 mask 外：
  │   │   └─ 标记为描边像素 (outline = 1.0)
  │   └─ 否则：
  │       └─ 保持原始颜色 (outline = 0.0)
  │
  └─ 如果当前像素在 mask 外：
      └─ 保持原始颜色 (outline = 0.0)

最终颜色 = mix(原始颜色, 描边颜色, outline)
```

## 改进方向

### 1. 平滑描边（Soft Outline）
当前实现是硬边缘，可以改为平滑过渡：
```glsl
// 计算到边缘的距离
float edgeDistance = 0.0;
for (int i = 0; i < 8; i++) {
    float neighborMask = texture2D(uMaskTexture, vTexCoord + offsets[i]).r;
    float diff = abs(maskValue - neighborMask);
    edgeDistance = max(edgeDistance, diff);
}
// 使用平滑函数
float outline = smoothstep(0.0, 0.3, edgeDistance);
```

### 2. 可调描边宽度
当前使用固定的 8 邻域，可以扩展为更大的采样范围：
```glsl
// 采样更多像素，形成更宽的描边
for (int y = -width; y <= width; y++) {
    for (int x = -width; x <= width; x++) {
        // 检查每个采样点
    }
}
```

### 3. 方向性描边
可以检测边缘方向，绘制有方向的描边效果。

### 4. 多通道 mask
当前只使用 mask 的红色通道，可以支持多通道或灰度 mask。

## 性能考虑

- **GPU 并行**：每个像素独立计算，充分利用 GPU 并行能力
- **早期退出**：找到第一个边缘像素后立即 break，减少不必要的采样
- **采样次数**：固定 8 次采样，性能稳定

## 使用示例

```cpp
// 设置描边参数
shader->SetUniform("uOutlineWidth", 2.0f);  // 2 像素宽
shader->SetUniform("uOutlineColor", 1.0f, 0.0f, 0.0f, 0.0f); // 红色描边
```

## 总结

这是一个简单而有效的边缘检测描边算法，适合实时渲染场景。通过检测 mask 图的边缘（从 mask 内到 mask 外的过渡），在边缘处绘制描边颜色，实现描边效果。
