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
#include <regex>
#include <functional>
#include <memory>

namespace SmartRenderer {

// Simple JSON parser helper functions
namespace {
    // Extract string value (handles escaped quotes and newlines)
    std::string ExtractString(const std::string& json, const std::string& key) {
        // Find the key and its value - handle whitespace and newlines
        std::string keyPattern = "\"" + key + "\"";
        size_t keyPos = json.find(keyPattern);
        if (keyPos == std::string::npos) {
            return "";
        }
        
        // Find the colon after the key
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) {
            return "";
        }
        
        // Skip whitespace after colon
        size_t valueStart = colonPos + 1;
        while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\n' || json[valueStart] == '\r' || json[valueStart] == '\t')) {
            valueStart++;
        }
        
        // Check if value starts with quote
        if (valueStart >= json.length() || json[valueStart] != '"') {
            return "";
        }
        
        valueStart++; // Skip opening quote
        
        std::string result;
        bool escaped = false;
        
        for (size_t i = valueStart; i < json.length(); ++i) {
            char c = json[i];
            
            if (escaped) {
                if (c == 'n') {
                    result += '\n';
                } else if (c == 't') {
                    result += '\t';
                } else if (c == 'r') {
                    result += '\r';
                } else if (c == '\\') {
                    result += '\\';
                } else if (c == '"') {
                    result += '"';
                } else {
                    result += c;
                }
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                // End of string
                break;
            } else {
                result += c;
            }
        }
        
        return result;
    }
    
    std::vector<std::string> ExtractStringArray(const std::string& json, const std::string& key) {
        std::vector<std::string> result;
        std::regex pattern("\"" + key + "\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch match;
        if (std::regex_search(json, match, pattern)) {
            std::string arrayContent = match[1].str();
            std::regex itemPattern("\"([^\"]+)\"");
            std::sregex_iterator iter(arrayContent.begin(), arrayContent.end(), itemPattern);
            std::sregex_iterator end;
            for (; iter != end; ++iter) {
                result.push_back((*iter)[1].str());
            }
        }
        return result;
    }
    
    // Extract shader configuration from node JSON
    std::pair<std::string, std::string> ExtractShader(const std::string& nodeJson) {
        std::string vertexShader, fragmentShader;
        
        // Find shader block - need to handle nested braces
        size_t shaderPos = nodeJson.find("\"shader\"");
        if (shaderPos == std::string::npos) {
            return {"", ""};
        }
        
        // Find the opening brace after "shader"
        size_t bracePos = nodeJson.find('{', shaderPos);
        if (bracePos == std::string::npos) {
            return {"", ""};
        }
        
        // Extract shader block content (handle nested braces)
        int braceCount = 0;
        size_t start = bracePos + 1;
        size_t pos = start;
        bool inString = false;
        bool escaped = false;
        
        while (pos < nodeJson.length()) {
            char c = nodeJson[pos];
            
            if (escaped) {
                escaped = false;
                pos++;
                continue;
            }
            
            if (c == '\\') {
                escaped = true;
                pos++;
                continue;
            }
            
            if (c == '"') {
                inString = !inString;
                pos++;
                continue;
            }
            
            if (inString) {
                pos++;
                continue;
            }
            
            if (c == '{') braceCount++;
            if (c == '}') {
                if (braceCount == 0) {
                    // Found the closing brace of shader block
                    std::string shaderJson = nodeJson.substr(start, pos - start);
                    // Use ExtractString which handles escape sequences correctly
                    vertexShader = ExtractString(shaderJson, "vertex");
                    fragmentShader = ExtractString(shaderJson, "fragment");
                    
                    #ifdef _DEBUG
                    if (!vertexShader.empty() || !fragmentShader.empty()) {
                        fprintf(stderr, "ExtractShader: Found shaders (vertex: %zu chars, fragment: %zu chars)\n",
                                vertexShader.length(), fragmentShader.length());
                    }
                    #endif
                    break;
                }
                braceCount--;
            }
            pos++;
        }
        
        return {vertexShader, fragmentShader};
    }
    
    // Extract viewport configuration
    struct ViewportConfig {
        int x = 0, y = 0, width = 0, height = 0;
        bool valid = false;
    };
    
    ViewportConfig ExtractViewport(const std::string& nodeJson);
    
    ViewportConfig ExtractViewport(const std::string& nodeJson) {
        ViewportConfig viewport;
        
        // Find viewport block - handle nested braces
        size_t viewportPos = nodeJson.find("\"viewport\"");
        if (viewportPos == std::string::npos) {
            #ifdef _DEBUG
            fprintf(stderr, "ExtractViewport: 'viewport' key not found\n");
            #endif
            return viewport;
        }
        
        #ifdef _DEBUG
        fprintf(stderr, "ExtractViewport: Found 'viewport' at position %zu\n", viewportPos);
        #endif
        
        // Find the opening brace after "viewport"
        size_t bracePos = nodeJson.find('{', viewportPos);
        if (bracePos == std::string::npos) {
            #ifdef _DEBUG
            fprintf(stderr, "ExtractViewport: Opening brace not found after 'viewport'\n");
            #endif
            return viewport;
        }
        
        #ifdef _DEBUG
        fprintf(stderr, "ExtractViewport: Found opening brace at position %zu\n", bracePos);
        #endif
        
        // Extract viewport block content (handle nested braces)
        int braceCount = 1; // Start at 1 because we already found the opening brace
        size_t start = bracePos + 1;
        size_t pos = start;
        bool inString = false;
        bool escaped = false;
        
        while (pos < nodeJson.length()) {
            char c = nodeJson[pos];
            
            if (escaped) {
                escaped = false;
                pos++;
                continue;
            }
            
            if (c == '\\') {
                escaped = true;
                pos++;
                continue;
            }
            
            if (c == '"') {
                inString = !inString;
                pos++;
                continue;
            }
            
            if (inString) {
                pos++;
                continue;
            }
            
            if (c == '{') braceCount++;
            if (c == '}') {
                braceCount--;
                if (braceCount == 0) {
                    // Found the closing brace of viewport object
                    std::string viewportJson = nodeJson.substr(start, pos - start);
                    
                    #ifdef _DEBUG
                    fprintf(stderr, "ExtractViewport: Extracted viewport JSON: %.100s\n", viewportJson.c_str());
                    #endif
                    
                    // Extract x, y, width, height using ExtractString (but these are numbers, not strings)
                    // Try direct number extraction instead
                    std::regex numPattern("(-?[0-9]+)");
                    std::sregex_iterator iter(viewportJson.begin(), viewportJson.end(), numPattern);
                    std::sregex_iterator end;
                    std::vector<int> values;
                    for (; iter != end; ++iter) {
                        values.push_back(std::stoi(iter->str()));
                    }
                    
                    #ifdef _DEBUG
                    fprintf(stderr, "ExtractViewport: Found %zu numbers\n", values.size());
                    #endif
                    
                    if (values.size() >= 4) {
                        viewport.x = values[0];
                        viewport.y = values[1];
                        viewport.width = values[2];
                        viewport.height = values[3];
                        viewport.valid = true;
                        
                        #ifdef _DEBUG
                        fprintf(stderr, "ExtractViewport: Extracted viewport (%d, %d, %d, %d)\n",
                                viewport.x, viewport.y, viewport.width, viewport.height);
                        #endif
                    } else {
                        #ifdef _DEBUG
                        fprintf(stderr, "ExtractViewport: Not enough numbers found (need 4, got %zu)\n", values.size());
                        #endif
                    }
                    break;
                }
            }
            pos++;
        }
        
        return viewport;
    }
    
    // Extract geometry configuration
    std::vector<float> ExtractGeometry(const std::string& nodeJson) {
        std::vector<float> vertices;
        
        // Find geometry block and vertices array
        std::regex geometryPattern("\"geometry\"\\s*:\\s*\\{[^\\}]*\"vertices\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch match;
        if (std::regex_search(nodeJson, match, geometryPattern)) {
            std::string verticesStr = match[1].str();
            
            // Parse float values (including negative)
            std::regex floatPattern("(-?[0-9]+\\.[0-9]+|-?[0-9]+)");
            std::sregex_iterator iter(verticesStr.begin(), verticesStr.end(), floatPattern);
            std::sregex_iterator end;
            for (; iter != end; ++iter) {
                vertices.push_back(std::stof(iter->str()));
            }
        }
        
        return vertices;
    }
    
    // Extract uniforms from node JSON
    std::map<std::string, std::vector<float>> ExtractUniforms(const std::string& nodeJson) {
        std::map<std::string, std::vector<float>> uniforms;
        
        // Find uniforms block
        std::regex uniformsBlockPattern("\"uniforms\"\\s*:\\s*\\{([^\\}]+)\\}");
        std::smatch uniformsMatch;
        if (std::regex_search(nodeJson, uniformsMatch, uniformsBlockPattern)) {
            std::string uniformsJson = uniformsMatch[1].str();
            
            // Extract each uniform
            std::regex uniformPattern("\"([^\"]+)\"\\s*:\\s*\\[([^\\]]+)\\]");
            std::sregex_iterator iter(uniformsJson.begin(), uniformsJson.end(), uniformPattern);
            std::sregex_iterator end;
            
            for (; iter != end; ++iter) {
                std::string uniformName = (*iter)[1].str();
                std::string valuesStr = (*iter)[2].str();
                
                // Parse float values
                std::vector<float> values;
                std::regex floatPattern("([0-9]+\\.[0-9]+|[0-9]+)");
                std::sregex_iterator valIter(valuesStr.begin(), valuesStr.end(), floatPattern);
                std::sregex_iterator valEnd;
                for (; valIter != valEnd; ++valIter) {
                    values.push_back(std::stof(valIter->str()));
                }
                uniforms[uniformName] = values;
            }
        }
        
        return uniforms;
    }
    
    std::vector<std::map<std::string, std::string>> ExtractNodeArray(const std::string& json) {
        std::vector<std::map<std::string, std::string>> nodes;
        
        // Find nodes array - need to handle nested objects and arrays
        // Look for "nodes": [ ... ] pattern, but need to handle nested braces
        size_t nodesPos = json.find("\"nodes\"");
        if (nodesPos == std::string::npos) {
            #ifdef _DEBUG
            fprintf(stderr, "ExtractNodeArray: 'nodes' key not found\n");
            #endif
            return nodes;
        }
        
        // Find the opening bracket after "nodes"
        size_t bracketPos = json.find('[', nodesPos);
        if (bracketPos == std::string::npos) {
            #ifdef _DEBUG
            fprintf(stderr, "ExtractNodeArray: '[' not found after 'nodes'\n");
            #endif
            return nodes;
        }
        
        #ifdef _DEBUG
        fprintf(stderr, "ExtractNodeArray: Found '[' at position %zu\n", bracketPos);
        #endif
        
        // Extract nodes array content (handle nested braces and strings)
        int bracketCount = 0;
        int braceCount = 0;
        size_t nodeStart = bracketPos + 1;
        size_t pos = nodeStart;
        bool inString = false;
        bool escaped = false;
        
        // Skip leading whitespace
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t')) {
            pos++;
        }
        nodeStart = pos;
        
        #ifdef _DEBUG
        fprintf(stderr, "ExtractNodeArray: Starting extraction at position %zu\n", nodeStart);
        fprintf(stderr, "ExtractNodeArray: Preview: %.100s\n", json.substr(nodeStart).c_str());
        #endif
        
        while (pos < json.length()) {
            char c = json[pos];
            
            if (escaped) {
                escaped = false;
                pos++;
                continue;
            }
            
            if (c == '\\') {
                escaped = true;
                pos++;
                continue;
            }
            
            if (c == '"') {
                inString = !inString;
                pos++;
                continue;
            }
            
            if (inString) {
                pos++;
                continue;
            }
            
            if (c == '[') {
                bracketCount++;
                pos++;
                continue;
            }
            
            if (c == ']') {
                if (bracketCount == 0) {
                    // Found the closing bracket of nodes array
                    // Extract last node if any
                    if (braceCount == 0 && pos > nodeStart) {
                        std::string nodeJson = json.substr(nodeStart, pos - nodeStart);
                        if (!nodeJson.empty() && nodeJson.find('{') != std::string::npos) {
                            std::map<std::string, std::string> node;
                            std::string type = ExtractString(nodeJson, "type");
                            std::string name = ExtractString(nodeJson, "name");
                            
                            if (!type.empty() && !name.empty()) {
                                node["type"] = type;
                                node["name"] = name;
                                node["json"] = nodeJson;
                                
                                std::vector<std::string> inputs = ExtractStringArray(nodeJson, "inputs");
                                std::vector<std::string> outputs = ExtractStringArray(nodeJson, "outputs");
                                for (size_t i = 0; i < inputs.size(); ++i) {
                                    node["input" + std::to_string(i)] = inputs[i];
                                }
                                for (size_t i = 0; i < outputs.size(); ++i) {
                                    node["output" + std::to_string(i)] = outputs[i];
                                }
                                nodes.push_back(node);
                                
                                #ifdef _DEBUG
                                fprintf(stderr, "ExtractNodeArray: Extracted node: %s (type: %s)\n", name.c_str(), type.c_str());
                                #endif
                            }
                        }
                    }
                    break;
                }
                bracketCount--;
                pos++;
                continue;
            }
            
            if (c == '{') {
                if (braceCount == 0) {
                    // Start of a new node
                    nodeStart = pos;
                    #ifdef _DEBUG
                    fprintf(stderr, "ExtractNodeArray: Found start of node at position %zu\n", nodeStart);
                    #endif
                }
                braceCount++;
                pos++;
                continue;
            }
            
            if (c == '}') {
                braceCount--;
                #ifdef _DEBUG
                if (braceCount <= 2 && bracketCount <= 1) {
                    fprintf(stderr, "ExtractNodeArray: Found '}' at position %zu, braceCount=%d, bracketCount=%d\n", pos, braceCount, bracketCount);
                }
                #endif
                if (braceCount == 0 && bracketCount == 0) {
                    // Found a complete node object
                    std::string nodeJson = json.substr(nodeStart, pos - nodeStart + 1);
                    #ifdef _DEBUG
                    fprintf(stderr, "ExtractNodeArray: Extracting node from position %zu to %zu (%zu chars)\n", nodeStart, pos, nodeJson.length());
                    #endif
                    
                    std::map<std::string, std::string> node;
                    std::string type = ExtractString(nodeJson, "type");
                    std::string name = ExtractString(nodeJson, "name");
                    
                    #ifdef _DEBUG
                    fprintf(stderr, "ExtractNodeArray: type='%s', name='%s'\n", type.c_str(), name.c_str());
                    fprintf(stderr, "ExtractNodeArray: nodeJson length=%zu, preview: %.300s\n", nodeJson.length(), nodeJson.c_str());
                    #endif
                    
                    if (!type.empty() && !name.empty()) {
                        node["type"] = type;
                        node["name"] = name;
                        node["json"] = nodeJson; // Store full JSON for shader extraction
                        
                        std::vector<std::string> inputs = ExtractStringArray(nodeJson, "inputs");
                        std::vector<std::string> outputs = ExtractStringArray(nodeJson, "outputs");
                        for (size_t i = 0; i < inputs.size(); ++i) {
                            node["input" + std::to_string(i)] = inputs[i];
                        }
                        for (size_t i = 0; i < outputs.size(); ++i) {
                            node["output" + std::to_string(i)] = outputs[i];
                        }
                        nodes.push_back(node);
                        
                        #ifdef _DEBUG
                        fprintf(stderr, "ExtractNodeArray: Extracted node: %s (type: %s)\n", name.c_str(), type.c_str());
                        #endif
                    }
                    
                    // Skip comma and whitespace to find next node
                    pos++;
                    while (pos < json.length() && (json[pos] == ',' || json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t')) {
                        pos++;
                    }
                    nodeStart = pos;
                    // Don't increment pos here, let the loop continue
                    continue;
                }
                pos++;
                continue;
            }
            
            pos++;
        }
        
        #ifdef _DEBUG
        fprintf(stderr, "ExtractNodeArray: Extracted %zu nodes\n", nodes.size());
        #endif
        
        return nodes;
    }
    
    // Extract connections array
    std::vector<std::map<std::string, std::string>> ExtractConnections(const std::string& json) {
        std::vector<std::map<std::string, std::string>> connections;
        
        std::regex connectionsPattern("\"connections\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch match;
        if (!std::regex_search(json, match, connectionsPattern)) {
            return connections;
        }
        
        std::string connectionsArray = match[1].str();
        
        // Extract each connection
        size_t pos = 0;
        while (pos < connectionsArray.length()) {
            if (connectionsArray[pos] == '{') {
                int braceCount = 0;
                size_t start = pos;
                while (pos < connectionsArray.length()) {
                    if (connectionsArray[pos] == '{') braceCount++;
                    if (connectionsArray[pos] == '}') {
                        braceCount--;
                        if (braceCount == 0) {
                            std::string connJson = connectionsArray.substr(start, pos - start + 1);
                            
                            std::map<std::string, std::string> conn;
                            conn["from"] = ExtractString(connJson, "from");
                            conn["output"] = ExtractString(connJson, "output");
                            conn["to"] = ExtractString(connJson, "to");
                            conn["input"] = ExtractString(connJson, "input");
                            connections.push_back(conn);
                            break;
                        }
                    }
                    pos++;
                }
            }
            pos++;
        }
        
        return connections;
    }
    
    std::pair<std::string, std::string> ExtractOutput(const std::string& json) {
        std::regex outputPattern("\"output\"\\s*:\\s*\\{[^\\}]*\"node\"\\s*:\\s*\"([^\"]+)\"[^\\}]*\"resource\"\\s*:\\s*\"([^\"]+)\"");
        std::smatch match;
        if (std::regex_search(json, match, outputPattern)) {
            return {match[1].str(), match[2].str()};
        }
        return {"", ""};
    }
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

bool RenderProtocol::LoadFromJSON(const std::string& json, Renderer* renderer) {
    m_renderGraph = std::make_unique<RenderGraph>();
    if (renderer) {
        m_renderer = renderer;
    }
    
    // Extract renderGraph object first (nodes are inside renderGraph)
    std::string renderGraphJson = json;
    size_t renderGraphPos = json.find("\"renderGraph\"");
    if (renderGraphPos != std::string::npos) {
        #ifdef _DEBUG
        fprintf(stderr, "LoadFromJSON: Found 'renderGraph' key at position %zu\n", renderGraphPos);
        #endif
        // Find the opening brace after "renderGraph"
        size_t bracePos = json.find('{', renderGraphPos);
        if (bracePos != std::string::npos) {
            #ifdef _DEBUG
            fprintf(stderr, "LoadFromJSON: Found opening brace at position %zu\n", bracePos);
            #endif
            // Extract renderGraph object content (handle nested braces)
            int braceCount = 0;
            size_t start = bracePos;
            size_t pos = start;
            bool inString = false;
            bool escaped = false;
            
            while (pos < json.length()) {
                char c = json[pos];
                
                if (escaped) {
                    escaped = false;
                    pos++;
                    continue;
                }
                
                if (c == '\\') {
                    escaped = true;
                    pos++;
                    continue;
                }
                
                if (c == '"') {
                    inString = !inString;
                    pos++;
                    continue;
                }
                
                if (inString) {
                    pos++;
                    continue;
                }
                
                if (c == '{') braceCount++;
                if (c == '}') {
                    braceCount--;
                    if (braceCount == 0) {
                        // Found the closing brace of renderGraph object
                        renderGraphJson = json.substr(start, pos - start + 1);
                        #ifdef _DEBUG
                        fprintf(stderr, "LoadFromJSON: Extracted renderGraph object (%zu chars)\n", renderGraphJson.length());
                        fprintf(stderr, "LoadFromJSON: renderGraph preview: %.200s\n", renderGraphJson.c_str());
                        #endif
                        break;
                    }
                }
                pos++;
            }
        } else {
            #ifdef _DEBUG
            fprintf(stderr, "LoadFromJSON: Opening brace not found after 'renderGraph'\n");
            #endif
        }
    } else {
        #ifdef _DEBUG
        fprintf(stderr, "LoadFromJSON: 'renderGraph' key not found, using full JSON\n");
        #endif
    }
    
    // Extract render graph name
    std::string graphName = ExtractString(renderGraphJson, "name");
    if (!graphName.empty()) {
        // Store graph name if needed
    }
    
    // Extract nodes from renderGraph object
    std::vector<std::map<std::string, std::string>> nodes = ExtractNodeArray(renderGraphJson);
    
    #ifdef _DEBUG
    fprintf(stderr, "Extracted %zu nodes from JSON\n", nodes.size());
    #endif
    
    // Create nodes using factory
    std::map<std::string, RenderNode*> nodeMap;
    for (const auto& nodeData : nodes) {
        auto typeIt = nodeData.find("type");
        auto nameIt = nodeData.find("name");
        
        if (typeIt == nodeData.end() || nameIt == nodeData.end()) {
            #ifdef _DEBUG
            fprintf(stderr, "Warning: Node missing type or name\n");
            #endif
            continue;
        }
        
        std::string type = typeIt->second;
        std::string name = nameIt->second;
        
        #ifdef _DEBUG
        fprintf(stderr, "Creating node: type=%s, name=%s\n", type.c_str(), name.c_str());
        #endif
        
        // Create node based on type (with shader support)
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
    std::vector<std::map<std::string, std::string>> connections = ExtractConnections(json);
    for (const auto& conn : connections) {
        auto fromIt = conn.find("from");
        auto outputIt = conn.find("output");
        auto toIt = conn.find("to");
        auto inputIt = conn.find("input");
        
        if (fromIt != conn.end() && outputIt != conn.end() && 
            toIt != conn.end() && inputIt != conn.end()) {
            m_renderGraph->Connect(
                fromIt->second, outputIt->second,
                toIt->second, inputIt->second
            );
        }
    }
    
    // Extract output - output node can be any RenderPass node
    auto output = ExtractOutput(json);
    if (!output.first.empty() && !output.second.empty()) {
        // Check if output node exists
        auto outputNodeIt = nodeMap.find(output.first);
        if (outputNodeIt != nodeMap.end()) {
            // Output node exists (should be a RenderPass), set it as output
            m_renderGraph->SetOutput(output.first, output.second);
            #ifdef _DEBUG
            fprintf(stderr, "Output set to node: %s, resource: %s\n", output.first.c_str(), output.second.c_str());
            #endif
        } else {
            // Output node not found
            #ifdef _DEBUG
            fprintf(stderr, "Warning: Output node '%s' not found, using last node as output\n", output.first.c_str());
            #endif
            // Use the last node as output
            if (!nodeMap.empty()) {
                auto lastNode = nodeMap.rbegin();
                m_renderGraph->SetOutput(lastNode->first, output.second);
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
}

std::unique_ptr<RenderNode> RenderProtocol::CreateNodeFromType(
    const std::string& type, 
    const std::map<std::string, std::string>& params) {
    
    // Extract shader configuration
    auto jsonIt = params.find("json");
    std::pair<std::string, std::string> shaderPair = {"", ""};
    std::map<std::string, std::vector<float>> uniforms;
    
    if (jsonIt != params.end()) {
        shaderPair = ExtractShader(jsonIt->second);
        uniforms = ExtractUniforms(jsonIt->second);
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
        
        // Extract viewport and geometry
        auto jsonIt = params.find("json");
        ViewportConfig viewport = {0, 0, 0, 0, false};
        std::vector<float> geometry;
        
        if (jsonIt != params.end()) {
            #ifdef _DEBUG
            fprintf(stderr, "CreateNodeFromType: Extracting viewport and geometry from JSON (length: %zu)\n", jsonIt->second.length());
            fprintf(stderr, "CreateNodeFromType: JSON preview: %.500s\n", jsonIt->second.c_str());
            fprintf(stderr, "CreateNodeFromType: Has 'viewport' key: %s\n", 
                    jsonIt->second.find("\"viewport\"") != std::string::npos ? "yes" : "no");
            #endif
            viewport = ExtractViewport(jsonIt->second);
            geometry = ExtractGeometry(jsonIt->second);
            #ifdef _DEBUG
            fprintf(stderr, "CreateNodeFromType: Viewport valid: %s\n", viewport.valid ? "yes" : "no");
            #endif
        } else {
            #ifdef _DEBUG
            fprintf(stderr, "CreateNodeFromType: No JSON found in params\n");
            #endif
        }
        
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
