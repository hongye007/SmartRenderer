#include "core/RenderNode.h"
#include "core/RenderContext.h"
#include "core/Renderer.h"
#include "math/MathTypes.h"
#include <vector>
#include <string>

namespace SmartRenderer {

class GeometryRenderNode : public RenderNode {
public:
    GeometryRenderNode(const std::string& name) : m_name(name) {}

    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer) return;

        // Set viewport
        renderer->SetViewport(0, 0, 1920, 1080);

        // Clear
        renderer->Clear(ClearFlags::All, Color::Black, 1.0f, 0);

        // Render geometry
        // In real implementation, would iterate through meshes and render them
    }

    std::vector<std::string> GetInputs() const override {
        return {"scene", "camera", "shadowMap"};
    }

    std::vector<std::string> GetOutputs() const override {
        return {"colorBuffer", "depthBuffer"};
    }

    std::string GetName() const override {
        return m_name;
    }

private:
    std::string m_name;
};

} // namespace SmartRenderer
