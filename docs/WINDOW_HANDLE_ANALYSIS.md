# WindowHandle 结构体分析

## 当前使用情况

### 1. 定义位置
- `include/platform/macos/MacOSWindow.h`: `struct WindowHandle { void* nsWindow; void* nsView; }`
- `include/platform/windows/WindowsWindow.h`: `struct WindowHandle { void* hwnd; void* hdc; }`
- `include/platform/Window.h`: 前向声明 `struct WindowHandle;`

### 2. 实际使用
- **macOS**: `m_handle.nsWindow = m_window` (GLFWwindow*), `m_handle.nsView = nullptr` (未使用)
- **Windows**: `m_handle.hwnd = m_window` (GLFWwindow*), `m_handle.hdc = nullptr` (未使用)
- **Android/iOS**: 未实现 `GetNativeHandle()`

### 3. 在 ANGLEContext 中的使用
```cpp
// 从 WindowHandle 中提取 GLFWwindow*
WindowHandle* handle = window->GetNativeHandle();
WindowHandle* macHandle = static_cast<WindowHandle*>(handle);
GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(macHandle->nsWindow);
```

## 问题分析

### 问题 1: 第二个字段未使用
- macOS: `nsView` 始终为 `nullptr`
- Windows: `hdc` 始终为 `nullptr`
- 这些字段完全没有实际用途

### 问题 2: 平台特定的结构体定义
- 每个平台都有自己的 WindowHandle 定义
- 但实际上都只存储 GLFWwindow* 指针
- 增加了不必要的复杂性

### 问题 3: 类型转换复杂
- 需要先获取 WindowHandle*，再从中提取 GLFWwindow*
- 多了一层间接访问

## 简化方案

### 方案 1: 统一 WindowHandle（推荐）⭐

**优点**：
- 保持接口统一
- 简化结构体定义
- 易于扩展（Android/iOS 可以添加自己的字段）

**实现**：
```cpp
// 统一的 WindowHandle 定义
struct WindowHandle {
    void* window;  // GLFWwindow* for macOS/Windows, platform-specific for others
};
```

### 方案 2: 直接返回 void*

**优点**：
- 最简单
- 减少一层间接访问

**缺点**：
- 失去类型安全性
- 难以扩展（如果 Android/iOS 需要多个字段）

**实现**：
```cpp
virtual void* GetNativeHandle() const = 0;
// 返回 GLFWwindow* (macOS/Windows) 或平台特定指针
```

### 方案 3: 使用模板或平台特定接口

**优点**：
- 类型安全
- 平台特定优化

**缺点**：
- 增加接口复杂度
- 需要平台特定的头文件

## 推荐方案

**推荐使用方案 1**：统一 WindowHandle 结构体

### 理由：
1. ✅ **保持接口统一**：所有平台使用相同的接口
2. ✅ **简化实现**：只需要一个字段存储窗口指针
3. ✅ **易于扩展**：如果 Android/iOS 需要多个字段，可以轻松添加
4. ✅ **类型安全**：比直接返回 void* 更安全
5. ✅ **向后兼容**：可以逐步迁移，不影响现有代码

### 实现步骤：
1. 在 `Window.h` 中定义统一的 `WindowHandle`
2. 更新 macOS 和 Windows 的实现
3. 简化 ANGLEContext 中的使用
