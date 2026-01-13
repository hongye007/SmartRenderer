# Platform 实现分析

## 代码对比分析

### 相似性（可以提取到基类）

#### 1. 构造函数和析构函数
- **完全相同**：都初始化 `m_initialized(false)` 和 `m_startTime(0.0)`
- **完全相同**：析构函数都调用 `Shutdown()`

#### 2. Initialize() 方法
- **完全相同**：检查 `m_initialized` 的早期返回
- **完全相同**：保存 `m_config`
- **完全相同**：时间初始化逻辑（使用 `std::chrono::high_resolution_clock`）
- **完全相同**：设置 `m_initialized = true` 并返回

#### 3. Shutdown() 方法
- **几乎相同**：Windows 有 `if (!m_initialized) return;` 检查
- **完全相同**：都设置 `m_initialized = false`

#### 4. CreateWindow() 方法
- **完全相同**：创建 WindowConfig
- **完全相同**：创建窗口对象（仅类型不同：WindowsWindow vs MacOSWindow）
- **完全相同**：错误处理和清理逻辑

#### 5. DestroyWindow() 方法
- **完全相同**：检查指针、调用 Destroy()、delete 对象

#### 6. GetInputManager() 和 GetFileSystem()
- **完全相同**：都返回 `nullptr`

#### 7. GetCurrentTime() 和 GetTimeSinceStart()
- **完全相同**：使用 `std::chrono::high_resolution_clock`
- **完全相同**：时间计算逻辑

#### 8. 成员变量
- **完全相同**：`PlatformConfig m_config`
- **完全相同**：`bool m_initialized`
- **完全相同**：`double m_startTime`

### 差异（需要平台特定实现）

#### 1. RunMainLoop()
- **Windows**: 实现了 Windows 消息循环（PeekMessage/DispatchMessage）
- **macOS**: 空实现（注释：macOS event loop would go here）

#### 2. RequestExit()
- **Windows**: 调用 `PostQuitMessage(0)`
- **macOS**: 空实现（注释：Request exit）

#### 3. GetScreenWidth() / GetScreenHeight()
- **Windows**: 使用 `GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN)` 获取实际屏幕尺寸
- **macOS**: 硬编码返回 `1920` / `1080`

#### 4. GetScreenDensity()
- **Windows**: 使用 DPI API (`GetDeviceCaps(LOGPIXELSX)`) 计算实际 DPI
- **macOS**: 硬编码返回 `1.0f`

#### 5. GetResourcePath()
- **Windows**: 获取可执行文件目录并拼接路径
- **macOS**: 直接返回相对路径（未实现）

#### 6. GetWritablePath()
- **Windows**: 使用 `SHGetFolderPathA(CSIDL_APPDATA)` 获取 AppData 目录
- **macOS**: 直接返回相对路径（未实现）

#### 7. GetType() / GetPlatformName() / IsMobile()
- **Windows**: 内联实现在头文件中
- **macOS**: 在 .cpp 文件中实现

### 代码统计

| 项目 | Windows | macOS | 相同部分 |
|------|---------|-------|----------|
| 总行数 | 148 | 116 | - |
| 完全相同的方法 | - | - | 8 个方法 |
| 部分相同的方法 | - | - | 2 个方法（Shutdown, Initialize） |
| 平台特定方法 | 5 个 | 5 个 | - |
| 可提取代码行数 | ~80 行 | ~80 行 | ~80 行 |

### 重构建议

#### 方案 1：创建 GLFWPlatformBase 基类（推荐）

由于两个平台都使用 GLFW，可以创建一个 `GLFWPlatformBase` 基类：

**提取到基类的内容：**
- 构造函数/析构函数
- `Initialize()` 的完整实现
- `Shutdown()` 的完整实现（统一检查逻辑）
- `CreateWindow()` / `DestroyWindow()` 的模板方法（使用 CRTP 或虚函数）
- `GetInputManager()` / `GetFileSystem()` 的默认实现
- `GetCurrentTime()` / `GetTimeSinceStart()` 的完整实现
- 成员变量

**保留在派生类的内容：**
- `RunMainLoop()` - 平台特定
- `RequestExit()` - 平台特定
- `GetScreenWidth/Height/Density()` - 平台特定
- `GetResourcePath()` / `GetWritablePath()` - 平台特定
- `GetType()` / `GetPlatformName()` / `IsMobile()` - 平台特定

**预期效果：**
- 减少代码重复：约 80 行
- 统一初始化/关闭逻辑
- 更容易维护

#### 方案 2：保持现状

如果平台差异较大，或者未来需要更多平台特定功能，可以保持现状。

### 当前问题

1. **macOS 的屏幕信息是硬编码的**：应该使用实际 API 获取
2. **macOS 的路径方法未实现**：应该使用 NSBundle/NSFileManager
3. **macOS 的事件循环未实现**：应该使用 NSApplication 或 GLFW 的事件循环
4. **代码重复**：约 80 行代码可以提取

### 建议

**短期**：创建 `GLFWPlatformBase` 基类，提取公共代码
**中期**：完善 macOS 的平台特定实现（屏幕信息、路径处理）
**长期**：考虑是否需要更通用的平台抽象层
