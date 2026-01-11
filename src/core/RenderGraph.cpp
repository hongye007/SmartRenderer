#include "core/RenderGraph.h"
#include "core/RenderNode.h"
#include "core/RenderContext.h"
#include <algorithm>
#include <queue>
#include <set>

namespace SmartRenderer {

RenderGraph::RenderGraph() : m_compiled(false), m_outputNodeName(""), m_outputResource("") {
}

RenderGraph::~RenderGraph() {
}

RenderNode* RenderGraph::AddNode(std::unique_ptr<RenderNode> node) {
    if (!node) {
        return nullptr;
    }
    std::string name = node->GetName();
    RenderNode* ptr = node.get();
    m_nodeMap[name] = ptr;
    m_nodes.push_back(std::move(node));
    m_compiled = false;
    return ptr;
}

void RenderGraph::Connect(const std::string& fromNode, const std::string& output,
                          const std::string& toNode, const std::string& input) {
    // Store connection information
    // In full implementation, would build dependency graph
    m_compiled = false;
}

void RenderGraph::SetOutput(const std::string& nodeName, const std::string& output) {
    // Set output node
    m_outputNodeName = nodeName;
    m_outputResource = output;
    m_compiled = false;
}

bool RenderGraph::Compile() {
    // Build execution order using topological sort
    // For now, use simple node order
    // In full implementation, would:
    // 1. Build dependency graph from connections
    // 2. Perform topological sort
    // 3. Validate no cycles exist
    // 4. Allocate resources
    
    m_executionOrder.clear();
    for (auto& node : m_nodes) {
        m_executionOrder.push_back(node->GetName());
    }
    
    m_compiled = true;
    return true;
}

void RenderGraph::Execute(RenderContext& context) {
    if (!m_compiled) {
        if (!Compile()) {
            return;
        }
    }

    // Clear the entire screen once at the start
    Renderer* renderer = context.GetRenderer();
    if (renderer) {
        renderer->SetViewport(0, 0, 1024, 768);
        renderer->Clear(ClearFlags::Color, Color(0.1f, 0.1f, 0.15f, 1.0f));
    }

    // If output node is set, execute all nodes except the output node itself
    // (output node is just a marker, its dependencies should be executed)
    std::string outputNodeName = m_outputNodeName;
    
    for (const auto& nodeName : m_executionOrder) {
        // Skip output node if it's set (it's just a marker)
        if (!outputNodeName.empty() && nodeName == outputNodeName) {
            #ifdef _DEBUG
            fprintf(stderr, "RenderGraph: Skipping output node '%s' (it's just a marker)\n", nodeName.c_str());
            #endif
            continue;
        }
        
        auto it = m_nodeMap.find(nodeName);
        if (it != m_nodeMap.end()) {
            #ifdef _DEBUG
            fprintf(stderr, "RenderGraph: Executing node '%s'\n", nodeName.c_str());
            #endif
            it->second->Execute(context);
        }
    }
}

RenderNode* RenderGraph::FindNode(const std::string& name) {
    auto it = m_nodeMap.find(name);
    if (it != m_nodeMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::unique_ptr<RenderGraph> RenderGraph::LoadFromFile(const std::string& path) {
    // In real implementation, would:
    // 1. Read JSON file
    // 2. Parse nodes and connections
    // 3. Create node instances
    // 4. Build graph
    
    return std::make_unique<RenderGraph>();
}

std::unique_ptr<RenderGraph> RenderGraph::LoadFromJSON(const std::string& json) {
    // In real implementation, would parse JSON and build graph
    return std::make_unique<RenderGraph>();
}

} // namespace SmartRenderer
