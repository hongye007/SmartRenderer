#include "core/RenderProtocol.h"
#include "core/RenderGraph.h"
#include "core/RenderNode.h"
#include "core/RenderContext.h"
#include "core/Renderer.h"
#include "rendering/Shader.h"
#include "rendering/VertexArray.h"
#include "rendering/Buffer.h"
#include "resource/AssetLoader.h"
#include <fstream>
#include <sstream>
#include <map>
#include <functional>
#include <memory>

// Use nlohmann/json library for JSON parsing
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace SmartRenderer {

// Viewport configuration structure
namespace {
    struct ViewportConfig {
        int x = 0, y = 0, width = 0, height = 0;
        bool valid = false;
    };
}

// Node factory - creates nodes based on type
class RenderNodeFactory {
public:
    using NodeCreator = std::function<std::unique_ptr<RenderNode>(const std::map<std::string, std::string>&)>;
    
    static RenderNodeFactory& GetInstance() {
        static RenderNodeFactory instance;
        return instance;
    }
    
    void RegisterNodeType(const std::string& type, NodeCreator creator) {
        m_creators[type] = creator;
    }
    
    std::unique_ptr<RenderNode> CreateNode(const std::string& type, const std::map<std::string, std::string>& params) {
        auto it = m_creators.find(type);
        if (it != m_creators.end()) {
            return it->second(params);
        }
        return nullptr;
    }
    
private:
    std::map<std::string, NodeCreator> m_creators;
};

RenderProtocol::RenderProtocol() {
    m_renderGraph = std::make_unique<RenderGraph>();
}

RenderProtocol::~RenderProtocol() {
}

bool RenderProtocol::LoadFromFile(const std::string& path) {
    try {
        std::string json = AssetLoader::LoadTextFile(path);
        if (json.empty()) {
            return false;
        }
        return LoadFromJSON(json, m_renderer);
    } catch (...) {
        return false;
    }
}

bool RenderProtocol::LoadFromJSON(const std::string& jsonStr, Renderer* renderer) {
    m_renderGraph = std::make_unique<RenderGraph>();
    if (renderer) {
        m_renderer = renderer;
    }
    
    try {
        // Parse JSON using nlohmann/json library
        json root = json::parse(jsonStr);
        
        // Extract renderGraph object
        if (!root.contains("renderGraph") || !root["renderGraph"].is_object()) {
            #ifdef _DEBUG
            fprintf(stderr, "LoadFromJSON: 'renderGraph' key not found or not an object\n");
            #endif
            return false;
        }
        
        json renderGraphObj = root["renderGraph"];
        
        // Extract render graph name
        std::string graphName;
        if (renderGraphObj.contains("name") && renderGraphObj["name"].is_string()) {
            graphName = renderGraphObj["name"].get<std::string>();
            #ifdef _DEBUG
            fprintf(stderr, "LoadFromJSON: Graph name: %s\n", graphName.c_str());
            #endif
        }
        
        // Extract nodes array
        if (!renderGraphObj.contains("nodes") || !renderGraphObj["nodes"].is_array()) {
            #ifdef _DEBUG
            fprintf(stderr, "LoadFromJSON: 'nodes' key not found or not an array\n");
            #endif
            return false;
        }
        
        json nodesArray = renderGraphObj["nodes"];
        
        #ifdef _DEBUG
        fprintf(stderr, "LoadFromJSON: Found %zu nodes\n", nodesArray.size());
        #endif
        
        // Create nodes using factory
        std::map<std::string, RenderNode*> nodeMap;
        for (const auto& nodeJson : nodesArray) {
            if (!nodeJson.is_object()) {
                #ifdef _DEBUG
                fprintf(stderr, "Warning: Node is not an object, skipping\n");
                #endif
                continue;
            }
            
            // Extract type and name
            if (!nodeJson.contains("type") || !nodeJson.contains("name")) {
                #ifdef _DEBUG
                fprintf(stderr, "Warning: Node missing type or name\n");
                #endif
                continue;
            }
            
            std::string type = nodeJson["type"].get<std::string>();
            std::string name = nodeJson["name"].get<std::string>();
            
            #ifdef _DEBUG
            fprintf(stderr, "Creating node: type=%s, name=%s\n", type.c_str(), name.c_str());
            #endif
            
            // Convert node JSON to map for CreateNodeFromType
            std::map<std::string, std::string> nodeData;
            nodeData["type"] = type;
            nodeData["name"] = name;
            
            // Store full JSON as string for shader extraction
            nodeData["json"] = nodeJson.dump();
            
            // Extract inputs and outputs
            if (nodeJson.contains("inputs") && nodeJson["inputs"].is_array()) {
                std::vector<std::string> inputs;
                for (const auto& input : nodeJson["inputs"]) {
                    if (input.is_string()) {
                        inputs.push_back(input.get<std::string>());
                    }
                }
                for (size_t i = 0; i < inputs.size(); ++i) {
                    nodeData["input" + std::to_string(i)] = inputs[i];
                }
            }
            
            if (nodeJson.contains("outputs") && nodeJson["outputs"].is_array()) {
                std::vector<std::string> outputs;
                for (const auto& output : nodeJson["outputs"]) {
                    if (output.is_string()) {
                        outputs.push_back(output.get<std::string>());
                    }
                }
                for (size_t i = 0; i < outputs.size(); ++i) {
                    nodeData["output" + std::to_string(i)] = outputs[i];
                }
            }
            
            // Create node based on type
            std::unique_ptr<RenderNode> node = CreateNodeFromType(type, nodeData);
            if (node) {
                RenderNode* nodePtr = m_renderGraph->AddNode(std::move(node));
                nodeMap[name] = nodePtr;
                #ifdef _DEBUG
                fprintf(stderr, "Successfully created node: %s\n", name.c_str());
                #endif
            } else {
                #ifdef _DEBUG
                fprintf(stderr, "Failed to create node: %s (type: %s)\n", name.c_str(), type.c_str());
                #endif
            }
        }
        
        #ifdef _DEBUG
        fprintf(stderr, "Total nodes created: %zu\n", nodeMap.size());
        for (const auto& pair : nodeMap) {
            fprintf(stderr, "  - %s\n", pair.first.c_str());
        }
        #endif
        
        // Extract and establish connections
        if (renderGraphObj.contains("connections") && renderGraphObj["connections"].is_array()) {
            json connectionsArray = renderGraphObj["connections"];
            for (const auto& connJson : connectionsArray) {
                if (!connJson.is_object()) continue;
                
                if (connJson.contains("from") && connJson.contains("output") &&
                    connJson.contains("to") && connJson.contains("input")) {
                    std::string from = connJson["from"].get<std::string>();
                    std::string output = connJson["output"].get<std::string>();
                    std::string to = connJson["to"].get<std::string>();
                    std::string input = connJson["input"].get<std::string>();
                    
                    m_renderGraph->Connect(from, output, to, input);
                }
            }
        }
        
        // Extract output - output node can be any RenderPass node
        if (renderGraphObj.contains("output") && renderGraphObj["output"].is_object()) {
            json outputObj = renderGraphObj["output"];
            if (outputObj.contains("node") && outputObj.contains("resource")) {
                std::string outputNode = outputObj["node"].get<std::string>();
                std::string outputResource = outputObj["resource"].get<std::string>();
                
                auto outputNodeIt = nodeMap.find(outputNode);
                if (outputNodeIt != nodeMap.end()) {
                    m_renderGraph->SetOutput(outputNode, outputResource);
                    #ifdef _DEBUG
                    fprintf(stderr, "Output set to node: %s, resource: %s\n", outputNode.c_str(), outputResource.c_str());
                    #endif
                } else {
                    #ifdef _DEBUG
                    fprintf(stderr, "Warning: Output node '%s' not found, using last node as output\n", outputNode.c_str());
                    #endif
                    if (!nodeMap.empty()) {
                        auto lastNode = nodeMap.rbegin();
                        m_renderGraph->SetOutput(lastNode->first, outputResource);
                    }
                }
            }
        } else {
            // No output specified, use last node
            if (!nodeMap.empty()) {
                auto lastNode = nodeMap.rbegin();
                m_renderGraph->SetOutput(lastNode->first, "finalOutput");
                #ifdef _DEBUG
                fprintf(stderr, "No output specified, using last node: %s\n", lastNode->first.c_str());
                #endif
            }
        }
        
        return m_renderGraph->Compile();
        
    } catch (const json::parse_error& e) {
        #ifdef _DEBUG
        fprintf(stderr, "JSON parse error: %s\n", e.what());
        #endif
        return false;
    } catch (const json::type_error& e) {
        #ifdef _DEBUG
        fprintf(stderr, "JSON type error: %s\n", e.what());
        #endif
        return false;
    } catch (const std::exception& e) {
        #ifdef _DEBUG
        fprintf(stderr, "Error loading JSON: %s\n", e.what());
        #endif
        return false;
    }
}

std::unique_ptr<RenderNode> RenderProtocol::CreateNodeFromType(
    const std::string& type, 
    const std::map<std::string, std::string>& params) {
    
    // Extract shader, viewport, and geometry configuration using nlohmann/json
    auto jsonIt = params.find("json");
    std::pair<std::string, std::string> shaderPair = {"", ""};
    std::map<std::string, std::vector<float>> uniforms;
    ViewportConfig viewport = {0, 0, 0, 0, false};
    std::vector<float> geometry;
    
    if (jsonIt != params.end()) {
        try {
            json nodeJson = json::parse(jsonIt->second);
            
            // Extract shader
            if (nodeJson.contains("shader") && nodeJson["shader"].is_object()) {
                json shaderObj = nodeJson["shader"];
                if (shaderObj.contains("vertex") && shaderObj["vertex"].is_string()) {
                    shaderPair.first = shaderObj["vertex"].get<std::string>();
                }
                if (shaderObj.contains("fragment") && shaderObj["fragment"].is_string()) {
                    shaderPair.second = shaderObj["fragment"].get<std::string>();
                }
            }
            
            // Extract uniforms
            if (nodeJson.contains("uniforms") && nodeJson["uniforms"].is_object()) {
                json uniformsObj = nodeJson["uniforms"];
                for (auto& [key, value] : uniformsObj.items()) {
                    if (value.is_array()) {
                        std::vector<float> values;
                        for (const auto& v : value) {
                            if (v.is_number()) {
                                values.push_back(v.get<float>());
                            }
                        }
                        uniforms[key] = values;
                    }
                }
            }
            
            // Extract viewport
            if (nodeJson.contains("viewport") && nodeJson["viewport"].is_object()) {
                json viewportObj = nodeJson["viewport"];
                if (viewportObj.contains("x") && viewportObj.contains("y") &&
                    viewportObj.contains("width") && viewportObj.contains("height")) {
                    viewport.x = viewportObj["x"].get<int>();
                    viewport.y = viewportObj["y"].get<int>();
                    viewport.width = viewportObj["width"].get<int>();
                    viewport.height = viewportObj["height"].get<int>();
                    viewport.valid = true;
                }
            }
            
            // Extract geometry
            if (nodeJson.contains("geometry") && nodeJson["geometry"].is_object()) {
                json geometryObj = nodeJson["geometry"];
                if (geometryObj.contains("vertices") && geometryObj["vertices"].is_array()) {
                    for (const auto& v : geometryObj["vertices"]) {
                        if (v.is_number()) {
                            geometry.push_back(v.get<float>());
                        }
                    }
                }
            }
        } catch (const json::parse_error& e) {
            #ifdef _DEBUG
            fprintf(stderr, "Error parsing node JSON: %s\n", e.what());
            #endif
        } catch (const std::exception& e) {
            #ifdef _DEBUG
            fprintf(stderr, "Error extracting node data: %s\n", e.what());
            #endif
        }
    }
    
    
    // Create basic node implementations
    if (type == "ClearPass" || type == "Clear") {
        class ClearNode : public RenderNode {
        public:
            ClearNode(const std::map<std::string, std::string>& p) : m_params(p) {}
            void Execute(RenderContext& context) override {
                Renderer* renderer = context.GetRenderer();
                if (renderer) {
                    renderer->Clear(ClearFlags::Color | ClearFlags::Depth, Color(0.1f, 0.1f, 0.15f, 1.0f));
                }
            }
            std::vector<std::string> GetInputs() const override { return {}; }
            std::vector<std::string> GetOutputs() const override { return {}; }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "clearPass";
            }
        private:
            std::map<std::string, std::string> m_params;
        };
        return std::make_unique<ClearNode>(params);
    }
    
    // TriangleGeometry node with shader support
    if (type == "TriangleGeometry") {
        class TriangleGeometryNode : public RenderNode {
        public:
            TriangleGeometryNode(Renderer* renderer, const std::map<std::string, std::string>& p, 
                                const std::string& vertexShader, const std::string& fragmentShader)
                : m_renderer(renderer), m_params(p) {
                if (renderer && !vertexShader.empty() && !fragmentShader.empty()) {
                    m_shader = renderer->CreateShader(vertexShader, fragmentShader);
                    
                    // Create triangle vertices
                    float vertices[] = {
                        -0.5f, -0.5f,  // Bottom left
                         0.5f, -0.5f,  // Bottom right
                         0.0f,  0.5f   // Top
                    };
                    
                    m_vertexBuffer = renderer->CreateBuffer(sizeof(vertices), vertices);
                    m_vertexArray = renderer->CreateVertexArray();
                    m_vertexArray->SetVertexBuffer(m_vertexBuffer, 0, 2, 2 * sizeof(float), 0);
                }
            }
            
            void Execute(RenderContext& context) override {
                // Geometry preparation - doesn't render here
            }
            
            std::vector<std::string> GetInputs() const override { return {}; }
            std::vector<std::string> GetOutputs() const override { 
                std::vector<std::string> outputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("output" + std::to_string(i));
                    if (it != m_params.end()) {
                        outputs.push_back(it->second);
                    }
                }
                return outputs;
            }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "triangleGeometry";
            }
            
            Shader* GetShader() const { return m_shader; }
            VertexArray* GetVertexArray() const { return m_vertexArray; }
            
        private:
            Renderer* m_renderer;
            std::map<std::string, std::string> m_params;
            Shader* m_shader = nullptr;
            Buffer* m_vertexBuffer = nullptr;
            VertexArray* m_vertexArray = nullptr;
        };
        return std::make_unique<TriangleGeometryNode>(m_renderer, params, shaderPair.first, shaderPair.second);
    }
    
    // RenderPass - Universal rendering node with shader, viewport, and geometry support
    if (type == "RenderPass") {
        class RenderPassNode : public RenderNode {
        public:
            RenderPassNode(Renderer* renderer, const std::map<std::string, std::string>& p,
                         const std::string& vertexShader, const std::string& fragmentShader,
                         const std::map<std::string, std::vector<float>>& uniforms,
                         const ViewportConfig& viewport, const std::vector<float>& geometry)
                : m_renderer(renderer), m_params(p), m_uniforms(uniforms), m_viewport(viewport) {
                std::string nodeName = p.find("name") != p.end() ? p.at("name") : "unknown";
                
                if (!renderer) {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: No renderer for node %s\n", nodeName.c_str());
                    #endif
                    return;
                }
                
                if (vertexShader.empty() || fragmentShader.empty()) {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: Empty shader source for node %s\n", nodeName.c_str());
                    fprintf(stderr, "  Vertex shader empty: %s\n", vertexShader.empty() ? "yes" : "no");
                    fprintf(stderr, "  Fragment shader empty: %s\n", fragmentShader.empty() ? "yes" : "no");
                    #endif
                    return;
                }
                
                #ifdef _DEBUG
                fprintf(stderr, "RenderPassNode: Creating shader for node %s\n", nodeName.c_str());
                fprintf(stderr, "  Vertex shader length: %zu, preview: %.80s\n", 
                        vertexShader.length(), vertexShader.c_str());
                fprintf(stderr, "  Fragment shader length: %zu, preview: %.80s\n", 
                        fragmentShader.length(), fragmentShader.c_str());
                #endif
                
                m_shader = renderer->CreateShader(vertexShader, fragmentShader);
                
                if (!m_shader) {
                    #ifdef _DEBUG
                    fprintf(stderr, "ERROR: RenderPassNode shader is null for node: %s\n", nodeName.c_str());
                    #endif
                    return;
                }
                
                if (!m_shader->IsValid()) {
                    #ifdef _DEBUG
                    fprintf(stderr, "ERROR: RenderPassNode shader is invalid for node: %s\n", nodeName.c_str());
                    fprintf(stderr, "  Shader ID: %u\n", m_shader->GetID());
                    #endif
                } else {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: Shader created successfully for node: %s (ID: %u)\n",
                            nodeName.c_str(), m_shader->GetID());
                    #endif
                }
                
                // Use provided geometry or default triangle
                std::vector<float> vertices = geometry;
                if (vertices.empty()) {
                    // Default triangle
                    vertices = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};
                }
                
                if (!vertices.empty() && vertices.size() >= 6) { // At least 3 vertices (2D)
                    m_vertexBuffer = renderer->CreateBuffer(vertices.size() * sizeof(float), vertices.data());
                    m_vertexArray = renderer->CreateVertexArray();
                    if (m_vertexArray && m_vertexBuffer) {
                        // Assume 2D positions (2 floats per vertex)
                        m_vertexArray->SetVertexBuffer(m_vertexBuffer, 0, 2, 2 * sizeof(float), 0);
                        m_vertexCount = static_cast<int>(vertices.size() / 2);
                        #ifdef _DEBUG
                        fprintf(stderr, "RenderPassNode: Created geometry with %d vertices for node %s\n",
                                m_vertexCount, nodeName.c_str());
                        #endif
                    }
                }
            }
            
            void Execute(RenderContext& context) override {
                Renderer* renderer = context.GetRenderer();
                if (!renderer) {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: No renderer\n");
                    #endif
                    return;
                }
                
                if (!m_shader || !m_shader->IsValid()) {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: Invalid shader for node %s\n", 
                            m_params.find("name") != m_params.end() ? m_params.at("name").c_str() : "unknown");
                    #endif
                    return;
                }
                
                if (!m_vertexArray) {
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: No vertex array for node %s\n",
                            m_params.find("name") != m_params.end() ? m_params.at("name").c_str() : "unknown");
                    #endif
                    return;
                }
                
                // Set viewport if configured
                if (m_viewport.valid) {
                    renderer->SetViewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: Set viewport for node %s: (%d, %d, %d, %d)\n",
                            m_params.find("name") != m_params.end() ? m_params.at("name").c_str() : "unknown",
                            m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
                    #endif
                    // Don't clear here - glClear clears entire framebuffer regardless of viewport
                    // Clearing will be done once at the start of frame
                } else {
                    // If no viewport configured, use full screen
                    renderer->SetViewport(0, 0, 1024, 768);
                    #ifdef _DEBUG
                    fprintf(stderr, "RenderPassNode: Set viewport for node %s: (0, 0, 1024, 768) [default]\n",
                            m_params.find("name") != m_params.end() ? m_params.at("name").c_str() : "unknown");
                    #endif
                    // Don't clear here either
                }
                
                renderer->BindShader(m_shader);
                
                // Set uniforms if needed
                // (shader already has hardcoded colors in fragment shader)
                
                renderer->BindVertexArray(m_vertexArray);
                renderer->DrawArrays(0, m_vertexCount);
                
                #ifdef _DEBUG
                fprintf(stderr, "RenderPassNode: Executed node %s, drew %d vertices\n",
                        m_params.find("name") != m_params.end() ? m_params.at("name").c_str() : "unknown",
                        m_vertexCount);
                #endif
            }
            
            std::vector<std::string> GetInputs() const override { 
                std::vector<std::string> inputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("input" + std::to_string(i));
                    if (it != m_params.end()) {
                        inputs.push_back(it->second);
                    }
                }
                return inputs;
            }
            std::vector<std::string> GetOutputs() const override { 
                std::vector<std::string> outputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("output" + std::to_string(i));
                    if (it != m_params.end()) {
                        outputs.push_back(it->second);
                    }
                }
                return outputs;
            }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "renderPass";
            }
            
        private:
            Renderer* m_renderer;
            std::map<std::string, std::string> m_params;
            std::map<std::string, std::vector<float>> m_uniforms;
            ViewportConfig m_viewport;
            Shader* m_shader = nullptr;
            Buffer* m_vertexBuffer = nullptr;
            VertexArray* m_vertexArray = nullptr;
            int m_vertexCount = 0;
        };
        
        // viewport and geometry are already extracted above using nlohmann/json
        return std::make_unique<RenderPassNode>(m_renderer, params, shaderPair.first, shaderPair.second, uniforms, viewport, geometry);
    }
    
    // ColorPass node with shader support (deprecated, use RenderPass instead)
    if (type == "ColorPass") {
        class ColorPassNode : public RenderNode {
        public:
            ColorPassNode(Renderer* renderer, const std::map<std::string, std::string>& p,
                         const std::string& vertexShader, const std::string& fragmentShader,
                         const std::map<std::string, std::vector<float>>& uniforms)
                : m_renderer(renderer), m_params(p), m_uniforms(uniforms) {
                if (renderer && !vertexShader.empty() && !fragmentShader.empty()) {
                    m_shader = renderer->CreateShader(vertexShader, fragmentShader);
                    
                    if (!m_shader || !m_shader->IsValid()) {
                        // Shader creation failed - this is a problem
                        #ifdef _DEBUG
                        fprintf(stderr, "Warning: ColorPassNode shader creation failed for node: %s\n", 
                                p.find("name") != p.end() ? p.at("name").c_str() : "unknown");
                        #endif
                    }
                    
                    // Create triangle vertices (same as geometry node)
                    float vertices[] = {
                        -0.5f, -0.5f,
                         0.5f, -0.5f,
                         0.0f,  0.5f
                    };
                    
                    m_vertexBuffer = renderer->CreateBuffer(sizeof(vertices), vertices);
                    m_vertexArray = renderer->CreateVertexArray();
                    if (m_vertexArray && m_vertexBuffer) {
                        m_vertexArray->SetVertexBuffer(m_vertexBuffer, 0, 2, 2 * sizeof(float), 0);
                    }
                }
            }
            
            void Execute(RenderContext& context) override {
                Renderer* renderer = context.GetRenderer();
                if (!renderer || !m_shader || !m_shader->IsValid()) {
                    // Debug: shader might not be valid
                    return;
                }
                
                // Don't clear here - let composite node handle clearing
                // renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
                
                renderer->BindShader(m_shader);
                
                // Set uniforms (shader already has hardcoded colors in fragment shader)
                // The fragment shader in JSON has hardcoded colors, so uniforms are optional
                
                if (m_vertexArray) {
                    renderer->BindVertexArray(m_vertexArray);
                    renderer->DrawArrays(0, 3);
                }
            }
            
            std::vector<std::string> GetInputs() const override { 
                std::vector<std::string> inputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("input" + std::to_string(i));
                    if (it != m_params.end()) {
                        inputs.push_back(it->second);
                    }
                }
                return inputs;
            }
            std::vector<std::string> GetOutputs() const override { 
                std::vector<std::string> outputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("output" + std::to_string(i));
                    if (it != m_params.end()) {
                        outputs.push_back(it->second);
                    }
                }
                return outputs;
            }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "colorPass";
            }
            
        private:
            Renderer* m_renderer;
            std::map<std::string, std::string> m_params;
            std::map<std::string, std::vector<float>> m_uniforms;
            Shader* m_shader = nullptr;
            Buffer* m_vertexBuffer = nullptr;
            VertexArray* m_vertexArray = nullptr;
        };
        return std::make_unique<ColorPassNode>(m_renderer, params, shaderPair.first, shaderPair.second, uniforms);
    }
    
    // Composite node - combines multiple render results
    if (type == "Composite") {
        class CompositeNode : public RenderNode {
        public:
            CompositeNode(Renderer* renderer, RenderGraph* graph, const std::map<std::string, std::string>& p)
                : m_renderer(renderer), m_graph(graph), m_params(p) {
                // Extract window dimensions from context or use defaults
                m_windowWidth = 1024;
                m_windowHeight = 768;
            }
            
            void Execute(RenderContext& context) override {
                Renderer* renderer = context.GetRenderer();
                if (!renderer || !m_graph) return;
                
                // Clear entire screen first
                renderer->Clear(ClearFlags::Color, Color(0.05f, 0.05f, 0.1f, 1.0f));
                
                // Find blue and red color nodes in the graph
                int halfWidth = m_windowWidth / 2;
                
                // Draw blue triangle on left half
                renderer->SetViewport(0, 0, halfWidth, m_windowHeight);
                auto blueNode = m_graph->FindNode("blueColorNode");
                if (blueNode) {
                    // Clear this viewport area
                    renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
                    blueNode->Execute(context);
                }
                
                // Draw red triangle on right half
                renderer->SetViewport(halfWidth, 0, halfWidth, m_windowHeight);
                auto redNode = m_graph->FindNode("redColorNode");
                if (redNode) {
                    // Clear this viewport area
                    renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
                    redNode->Execute(context);
                }
                
                // Reset viewport to full screen
                renderer->SetViewport(0, 0, m_windowWidth, m_windowHeight);
            }
            
            std::vector<std::string> GetInputs() const override { 
                std::vector<std::string> inputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("input" + std::to_string(i));
                    if (it != m_params.end()) {
                        inputs.push_back(it->second);
                    }
                }
                return inputs;
            }
            std::vector<std::string> GetOutputs() const override { 
                std::vector<std::string> outputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("output" + std::to_string(i));
                    if (it != m_params.end()) {
                        outputs.push_back(it->second);
                    }
                }
                return outputs;
            }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "compositeNode";
            }
            
        private:
            Renderer* m_renderer;
            RenderGraph* m_graph;
            std::map<std::string, std::string> m_params;
            int m_windowWidth;
            int m_windowHeight;
        };
        return std::make_unique<CompositeNode>(m_renderer, m_renderGraph.get(), params);
    }
    
    if (type == "GeometryPass" || type == "Geometry") {
        class GeometryNode : public RenderNode {
        public:
            GeometryNode(const std::map<std::string, std::string>& p) : m_params(p) {}
            void Execute(RenderContext& context) override {
                // Geometry rendering placeholder
            }
            std::vector<std::string> GetInputs() const override { return {}; }
            std::vector<std::string> GetOutputs() const override { 
                std::vector<std::string> outputs;
                for (size_t i = 0; i < 10; ++i) {
                    auto it = m_params.find("output" + std::to_string(i));
                    if (it != m_params.end()) {
                        outputs.push_back(it->second);
                    }
                }
                return outputs;
            }
            std::string GetName() const override { 
                auto it = m_params.find("name");
                return (it != m_params.end()) ? it->second : "geometryPass";
            }
        private:
            std::map<std::string, std::string> m_params;
        };
        return std::make_unique<GeometryNode>(params);
    }
    
    // Default: return empty node
    class DefaultNode : public RenderNode {
    public:
        DefaultNode(const std::map<std::string, std::string>& p) : m_params(p) {}
        void Execute(RenderContext& context) override {}
        std::vector<std::string> GetInputs() const override { return {}; }
        std::vector<std::string> GetOutputs() const override { return {}; }
        std::string GetName() const override { 
            auto it = m_params.find("name");
            return (it != m_params.end()) ? it->second : "defaultNode";
        }
    private:
        std::map<std::string, std::string> m_params;
    };
    return std::make_unique<DefaultNode>(params);
}

void RenderProtocol::UpdateConfig(const RenderProtocolConfig& config) {
    m_config = config;
    
    // Rebuild render graph based on config
    m_renderGraph = std::make_unique<RenderGraph>();
    
    // Add nodes based on configuration
    if (m_config.features.shadows) {
        // Add shadow pass node
    }
    
    // Add geometry pass
    // Add post-processing nodes if enabled
    
    m_renderGraph->Compile();
}

void RenderProtocol::Execute(RenderContext& context) {
    if (m_renderGraph) {
        m_renderGraph->Execute(context);
    }
}

} // namespace SmartRenderer
