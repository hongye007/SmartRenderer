#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace SmartRenderer {

class RenderContext;
class RenderNode;
class RenderResource;

class RenderGraph {
public:
    RenderGraph();
    ~RenderGraph();

    RenderNode* AddNode(std::unique_ptr<RenderNode> node);
    void Connect(const std::string& fromNode, const std::string& output,
                 const std::string& toNode, const std::string& input);
    void SetOutput(const std::string& nodeName, const std::string& output);

    bool Compile();
    void Execute(RenderContext& context);
    
    RenderNode* FindNode(const std::string& name);

    static std::unique_ptr<RenderGraph> LoadFromFile(const std::string& path);
    static std::unique_ptr<RenderGraph> LoadFromJSON(const std::string& json);

private:
    std::vector<std::unique_ptr<RenderNode>> m_nodes;
    std::map<std::string, RenderNode*> m_nodeMap;
    std::vector<std::string> m_executionOrder;
    std::map<std::string, RenderResource*> m_resources;
    std::string m_outputNodeName;
    std::string m_outputResource;
    bool m_compiled;
};

} // namespace SmartRenderer
