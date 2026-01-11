#pragma once

#include "RenderGraph.h"
#include "RenderNode.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace SmartRenderer {

struct RenderProtocolConfig {
    enum class Mode {
        Forward,
        Deferred,
        ForwardPlus,
        Custom
    } mode = Mode::Forward;

    struct Features {
        bool shadows = true;
        bool reflections = false;
        bool postProcessing = true;
        bool antiAliasing = true;
        bool hdr = true;
    } features;

    std::vector<std::string> postProcessPipeline = {
        "bloom",
        "toneMapping",
        "fxaa"
    };

    struct RenderTarget {
        int width = 1920;
        int height = 1080;
        bool hdr = true;
        int msaa = 0;
    } renderTarget;
};

class Renderer;

class RenderProtocol {
public:
    RenderProtocol();
    ~RenderProtocol();

    bool LoadFromFile(const std::string& path);
    bool LoadFromJSON(const std::string& json, Renderer* renderer = nullptr);
    RenderGraph* GetRenderGraph() { return m_renderGraph.get(); }
    void UpdateConfig(const RenderProtocolConfig& config);
    void Execute(RenderContext& context);
    void SetRenderer(Renderer* renderer) { m_renderer = renderer; }

private:
    std::unique_ptr<RenderGraph> m_renderGraph;
    RenderProtocolConfig m_config;
    Renderer* m_renderer = nullptr;
    
    std::unique_ptr<RenderNode> CreateNodeFromType(
        const std::string& type, 
        const std::map<std::string, std::string>& params);
};

} // namespace SmartRenderer
