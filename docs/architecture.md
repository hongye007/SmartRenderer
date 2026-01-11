# SmartRenderer Architecture

## Overview

SmartRenderer is a cross-platform rendering engine built with a layered architecture.

## Architecture Layers

1. **Application Layer**: User code
2. **Engine API Layer**: Renderer, Platform, Resource interfaces
3. **Core Systems**: Render Pipeline, Math Library, Asset Manager
4. **Platform Abstraction Layer**: Platform-specific implementations
5. **Rendering Backend**: OpenGL ES → ANGLE → Platform Native API

## Key Components

- **Platform Abstraction**: Unified API across Windows, macOS, iOS, Android
- **ANGLE Integration**: Automatic conversion to platform-native graphics APIs
- **Render Graph System**: Configurable rendering pipeline
- **Resource Management**: Centralized asset loading and management

## For more details, see the main README.md
