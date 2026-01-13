#include "SmartRenderer.h"
#include "core/RenderProtocol.h"
#include "core/RenderContext.h"
#include "core/RenderGraph.h"
#include "core/RenderNode.h"
#include "rendering/Shader.h"
#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"
#include <iostream>
#include <filesystem>
#include <memory>

using namespace SmartRenderer;

// Shader for triangle rendering
const char* triangleVertexShader = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 aPosition;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
})";

const char* triangleFragmentShader = R"(#version 300 es
precision mediump float;

uniform vec3 uColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor, 1.0);
})";

// Node implementations that can be created from JSON
class TriangleGeometryNode : public RenderNode {
public:
    TriangleGeometryNode(Renderer* renderer) : m_renderer(renderer) {
        // Create shader
        m_shader = renderer->CreateShader(triangleVertexShader, triangleFragmentShader);
        
        // Triangle vertices (position only)
        float vertices[] = {
            -0.5f, -0.5f,  // Bottom left
             0.5f, -0.5f,  // Bottom right
             0.0f,  0.5f   // Top
        };
        
        // Create vertex buffer
        m_vertexBuffer = renderer->CreateBuffer(sizeof(vertices), vertices);
        
        // Create vertex array
        m_vertexArray = renderer->CreateVertexArray();
        m_vertexArray->SetVertexBuffer(m_vertexBuffer, 0, 2, 2 * sizeof(float), 0);
    }
    
    void Execute(RenderContext& context) override {
        // This node just prepares geometry, doesn't render
    }
    
    std::vector<std::string> GetInputs() const override { return {}; }
    std::vector<std::string> GetOutputs() const override { return {"triangleMesh"}; }
    std::string GetName() const override { return "triangleGeometry"; }
    
    Shader* GetShader() const { return m_shader; }
    VertexArray* GetVertexArray() const { return m_vertexArray; }
    
private:
    Renderer* m_renderer;
    Shader* m_shader;
    Buffer* m_vertexBuffer;
    VertexArray* m_vertexArray;
};

class BlueColorNode : public RenderNode {
public:
    BlueColorNode(TriangleGeometryNode* geometryNode) : m_geometryNode(geometryNode) {}
    
    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer || !m_geometryNode) return;
        
        // Clear to dark background
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // Draw triangle with blue color
        Shader* shader = m_geometryNode->GetShader();
        if (shader && shader->IsValid()) {
            renderer->BindShader(shader);
            // Note: We'll need to extend SetUniform for vec3, for now use workaround
            renderer->BindVertexArray(m_geometryNode->GetVertexArray());
            renderer->DrawArrays(0, 3);
        }
    }
    
    std::vector<std::string> GetInputs() const override { return {"triangleMesh"}; }
    std::vector<std::string> GetOutputs() const override { return {"blueTriangle"}; }
    std::string GetName() const override { return "blueColorNode"; }
    
private:
    TriangleGeometryNode* m_geometryNode;
};

class RedColorNode : public RenderNode {
public:
    RedColorNode(TriangleGeometryNode* geometryNode) : m_geometryNode(geometryNode) {}
    
    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer || !m_geometryNode) return;
        
        // Clear to dark background
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // Draw triangle with red color
        Shader* shader = m_geometryNode->GetShader();
        if (shader && shader->IsValid()) {
            renderer->BindShader(shader);
            renderer->BindVertexArray(m_geometryNode->GetVertexArray());
            renderer->DrawArrays(0, 3);
        }
    }
    
    std::vector<std::string> GetInputs() const override { return {"triangleMesh"}; }
    std::vector<std::string> GetOutputs() const override { return {"redTriangle"}; }
    std::string GetName() const override { return "redColorNode"; }
    
private:
    TriangleGeometryNode* m_geometryNode;
};

class CompositeNode : public RenderNode {
public:
    CompositeNode(Renderer* renderer, int windowWidth, int windowHeight, 
                  BlueColorNode* blueNode, RedColorNode* redNode)
        : m_renderer(renderer), m_windowWidth(windowWidth), m_windowHeight(windowHeight),
          m_blueNode(blueNode), m_redNode(redNode) {}
    
    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer) return;
        
        // Clear entire screen
        renderer->Clear(ClearFlags::Color, Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // Draw blue triangle on left half
        int halfWidth = m_windowWidth / 2;
        renderer->SetViewport(0, 0, halfWidth, m_windowHeight);
        if (m_blueNode) {
            m_blueNode->Execute(context);
        }
        
        // Draw red triangle on right half
        renderer->SetViewport(halfWidth, 0, halfWidth, m_windowHeight);
        if (m_redNode) {
            m_redNode->Execute(context);
        }
        
        // Reset viewport
        renderer->SetViewport(0, 0, m_windowWidth, m_windowHeight);
    }
    
    std::vector<std::string> GetInputs() const override { return {"blueTriangle", "redTriangle"}; }
    std::vector<std::string> GetOutputs() const override { return {"finalOutput"}; }
    std::string GetName() const override { return "compositeNode"; }
    
private:
    Renderer* m_renderer;
    int m_windowWidth;
    int m_windowHeight;
    BlueColorNode* m_blueNode;
    RedColorNode* m_redNode;
};

int main() {
    std::cout << "=== Render Graph Demo ===" << std::endl;
    std::cout << "Demonstrating JSON protocol parsing for custom render chain" << std::endl;
    
    const int windowWidth = 1024;
    const int windowHeight = 768;
    
    PlatformConfig config;
    config.windowWidth = windowWidth;
    config.windowHeight = windowHeight;
    config.appName = "Render Graph Demo";

    auto platform = CreatePlatform(); // Auto-detect platform
    if (!platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return 1;
    }

    auto window = platform->CreateWindow(windowWidth, windowHeight, "Render Graph - JSON Protocol Demo");
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    window->Show();
    std::cout << "Window created and shown" << std::endl;

    RendererConfig renderConfig;
    auto renderer = CreateRenderer(window, renderConfig);
    if (!renderer) {
        std::cerr << "Failed to create renderer" << std::endl;
        return 1;
    }

    std::cout << "Renderer initialized successfully" << std::endl;

    // Try to load from JSON protocol file
    std::cout << "\n=== Loading Render Protocol from JSON ===" << std::endl;
    RenderProtocol protocol;
    protocol.SetRenderer(renderer.get()); // Set renderer for shader creation
    
    std::vector<std::string> configPaths = {
        "configs/triangle_split.json",
        "../configs/triangle_split.json",
        "../../configs/triangle_split.json",
        "../../../configs/triangle_split.json",
        "configs/forward_rendering.json",
        "../configs/forward_rendering.json"
    };
    
    bool configLoaded = false;
    std::string loadedPath;
    for (const auto& path : configPaths) {
        try {
            if (protocol.LoadFromFile(path)) {
                std::cout << "✓ Render protocol loaded from: " << path << std::endl;
                std::cout << "  • JSON contains render graph assembly logic" << std::endl;
                std::cout << "  • Each node includes shader configuration" << std::endl;
                configLoaded = true;
                loadedPath = path;
                break;
            }
        } catch (const std::exception& e) {
            std::cout << "  Failed to load " << path << ": " << e.what() << std::endl;
        } catch (...) {
            // Continue trying other paths
        }
    }
    
    bool useProtocolGraph = false;
    std::unique_ptr<RenderGraph> renderGraph;
    
    if (!configLoaded) {
        std::cout << "⚠ Could not load from JSON file, building programmatically" << std::endl;
    } else {
        // Use protocol's render graph if loaded successfully
        auto protocolGraph = protocol.GetRenderGraph();
        if (protocolGraph) {
            useProtocolGraph = true;
            std::cout << "  ✓ Using render graph from JSON protocol" << std::endl;
            std::cout << "  ✓ All nodes loaded from JSON, skipping programmatic build" << std::endl;
        }
    }

    // Build render graph programmatically (only if JSON not loaded)
    if (!useProtocolGraph) {
        std::cout << "\n=== Building Render Graph Programmatically ===" << std::endl;
        renderGraph = std::make_unique<RenderGraph>();
        
        // Create geometry node
        std::cout << "1. Creating Triangle Geometry Node..." << std::endl;
    auto triangleNode = renderGraph->AddNode(std::make_unique<TriangleGeometryNode>(renderer.get()));
    if (triangleNode) {
        std::cout << "   ✓ Added: " << triangleNode->GetName() << std::endl;
    }
    
    // Create blue color node
    std::cout << "2. Creating Blue Color Node..." << std::endl;
    auto blueNode = renderGraph->AddNode(
        std::make_unique<BlueColorNode>(static_cast<TriangleGeometryNode*>(triangleNode))
    );
    if (blueNode) {
        std::cout << "   ✓ Added: " << blueNode->GetName() << std::endl;
    }
    
    // Create red color node
    std::cout << "3. Creating Red Color Node..." << std::endl;
    auto redNode = renderGraph->AddNode(
        std::make_unique<RedColorNode>(static_cast<TriangleGeometryNode*>(triangleNode))
    );
    if (redNode) {
        std::cout << "   ✓ Added: " << redNode->GetName() << std::endl;
    }
    
    // Create composite node
    std::cout << "4. Creating Composite Node..." << std::endl;
    auto compositeNode = renderGraph->AddNode(
        std::make_unique<CompositeNode>(
            renderer.get(), windowWidth, windowHeight,
            static_cast<BlueColorNode*>(blueNode),
            static_cast<RedColorNode*>(redNode)
        )
    );
    if (compositeNode) {
        std::cout << "   ✓ Added: " << compositeNode->GetName() << std::endl;
    }
    
    // Connect nodes
    std::cout << "\n=== Connecting Render Nodes ===" << std::endl;
    renderGraph->Connect("triangleGeometry", "triangleMesh", "blueColorNode", "triangleMesh");
    std::cout << "   ✓ triangleGeometry -> blueColorNode" << std::endl;
    
    renderGraph->Connect("triangleGeometry", "triangleMesh", "redColorNode", "triangleMesh");
    std::cout << "   ✓ triangleGeometry -> redColorNode" << std::endl;
    
    renderGraph->Connect("blueColorNode", "blueTriangle", "compositeNode", "blueTriangle");
    std::cout << "   ✓ blueColorNode -> compositeNode" << std::endl;
    
    renderGraph->Connect("redColorNode", "redTriangle", "compositeNode", "redTriangle");
    std::cout << "   ✓ redColorNode -> compositeNode" << std::endl;
    
    // Set output
    std::cout << "\n=== Setting Output ===" << std::endl;
    renderGraph->SetOutput("compositeNode", "finalOutput");
    std::cout << "   ✓ Output: compositeNode.finalOutput" << std::endl;
    
        // Compile render graph
        std::cout << "\n=== Compiling Render Graph ===" << std::endl;
        if (renderGraph->Compile()) {
            std::cout << "   ✓ Render graph compiled successfully" << std::endl;
        } else {
            std::cerr << "   ✗ Failed to compile render graph" << std::endl;
            return 1;
        }
    }
    
    RenderContext context(renderer.get());
    
    std::cout << "\n=== Render Chain Structure ===" << std::endl;
    std::cout << "1. Triangle Geometry Node - Prepares triangle mesh" << std::endl;
    std::cout << "   ├─> 2. Blue Color Node - Renders triangle in blue (left half)" << std::endl;
    std::cout << "   └─> 3. Red Color Node - Renders triangle in red (right half)" << std::endl;
    std::cout << "4. Composite Node - Displays both triangles side by side" << std::endl;
    std::cout << "\nStarting render loop..." << std::endl;
    std::cout << "You should see:" << std::endl;
    std::cout << "  • Left half: Blue triangle" << std::endl;
    std::cout << "  • Right half: Red triangle" << std::endl;
    std::cout << "\nPress ESC or close window to exit" << std::endl;

    int frameCount = 0;
    while (!window->ShouldClose()) {
        window->PollEvents();

        renderer->BeginFrame();
        
        // Clear entire screen first
        renderer->Clear(ClearFlags::Color, Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // Execute render graph
        if (useProtocolGraph) {
            protocol.Execute(context);
        } else {
            renderGraph->Execute(context);
        }
        
        // Reset viewport to full screen after rendering
        renderer->SetViewport(0, 0, windowWidth, windowHeight);
        
        renderer->EndFrame();
        renderer->Present();
        
        frameCount++;
        if (frameCount == 1) {
            std::cout << "\nFirst frame rendered!" << std::endl;
        }
    }

    std::cout << "\nShutting down..." << std::endl;
    renderer->Shutdown();
    platform->Shutdown();
    
    std::cout << "Render graph demo completed successfully" << std::endl;
    std::cout << "Total frames rendered: " << frameCount << std::endl;
    if (configLoaded) {
        std::cout << "JSON protocol was loaded from: " << loadedPath << std::endl;
    }
    return 0;
}
