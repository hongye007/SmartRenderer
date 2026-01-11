# ANGLE Integration Guide

## What is ANGLE?

ANGLE (Almost Native Graphics Layer Engine) is a graphics abstraction layer developed by Google. It translates OpenGL ES API calls to:
- **DirectX 11/12** on Windows
- **Metal** on macOS/iOS
- **Vulkan** on supported platforms
- **OpenGL** as fallback

## Installation Options

### Option 1: Automated Download (Recommended)

Run the PowerShell script:
```powershell
cd third_party
.\download_angle.ps1
```

### Option 2: Manual Installation

1. **Download ANGLE**
   - Visit: https://github.com/google/angle/releases
   - Or build from source: https://github.com/google/angle

2. **Extract to `third_party/angle/`**
   ```
   third_party/
   └── angle/
       ├── include/
       │   ├── EGL/
       │   ├── GLES2/
       │   ├── GLES3/
       │   └── KHR/
       └── lib/
           ├── libEGL.dll
           ├── libEGL.lib
           ├── libGLESv2.dll
           └── libGLESv2.lib
   ```

### Option 3: Build from Source

```bash
# Clone ANGLE repository
git clone https://chromium.googlesource.com/angle/angle

# Follow ANGLE build instructions
# https://github.com/google/angle/blob/main/doc/DevSetup.md
```

## Required Files

### Include Files (Headers)
- `EGL/egl.h` - EGL core API
- `EGL/eglext.h` - EGL extensions
- `EGL/eglplatform.h` - Platform definitions
- `GLES3/gl3.h` - OpenGL ES 3.0 API
- `GLES3/gl3platform.h` - Platform definitions
- `KHR/khrplatform.h` - Khronos platform types

### Library Files (Windows x64)
- `libEGL.lib` - EGL import library
- `libGLESv2.lib` - OpenGL ES import library
- `libEGL.dll` - EGL runtime library
- `libGLESv2.dll` - OpenGL ES runtime library

## CMake Integration

The CMakeLists.txt will automatically detect ANGLE if it's in `third_party/angle/`:

```cmake
if(USE_ANGLE)
    find_path(ANGLE_INCLUDE_DIR NAMES EGL/egl.h
        PATHS ${CMAKE_SOURCE_DIR}/third_party/angle/include
        NO_DEFAULT_PATH)
    
    find_library(ANGLE_EGL_LIBRARY NAMES libEGL EGL
        PATHS ${CMAKE_SOURCE_DIR}/third_party/angle/lib
        NO_DEFAULT_PATH)
    
    find_library(ANGLE_GLES_LIBRARY NAMES libGLESv2 GLESv2
        PATHS ${CMAKE_SOURCE_DIR}/third_party/angle/lib
        NO_DEFAULT_PATH)
endif()
```

## Verification

After installation, verify the structure:

```powershell
# Check if files exist
Test-Path third_party/angle/include/EGL/egl.h
Test-Path third_party/angle/lib/libEGL.lib
Test-Path third_party/angle/lib/libGLESv2.lib
```

## Alternative: Using ANGLE Binaries from Chrome

You can extract ANGLE DLLs from Chrome installation:
1. Locate Chrome installation (usually `C:\Program Files\Google\Chrome\Application\<version>\`)
2. Copy `libEGL.dll` and `libGLESv2.dll`
3. Generate import libraries using `lib.exe` tool from Visual Studio

```cmd
lib /def:libEGL.def /out:libEGL.lib /machine:x64
lib /def:libGLESv2.def /out:libGLESv2.lib /machine:x64
```

## Troubleshooting

### Missing DLLs at Runtime
Copy `libEGL.dll` and `libGLESv2.dll` to your executable directory or add ANGLE lib directory to PATH.

### Link Errors
Ensure CMake found the correct libraries:
```
ANGLE_INCLUDE_DIR: <path>/third_party/angle/include
ANGLE_EGL_LIBRARY: <path>/third_party/angle/lib/libEGL.lib
ANGLE_GLES_LIBRARY: <path>/third_party/angle/lib/libGLESv2.lib
```

### Header Not Found
Check that `third_party/angle/include/` contains EGL/, GLES2/, GLES3/ subdirectories.

## More Information

- ANGLE GitHub: https://github.com/google/angle
- ANGLE Documentation: https://chromium.googlesource.com/angle/angle/+/main/README.md
- Building ANGLE: https://github.com/google/angle/blob/main/doc/DevSetup.md
