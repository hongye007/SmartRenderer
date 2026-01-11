#include "core/RenderNode.h"
#include "core/RenderContext.h"
#include <vector>
#include <string>

namespace SmartRenderer {

class ShadowRenderNode : public RenderNode {
public:
    ShadowRenderNode(const std::string& name) : m_name(name) {}

    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer) return;

        // Render shadow map from light's perspective
        // In real implementation, would set up light view matrix and render scene
    }

    std::vector<std::string> GetInputs() const override {
        return {"scene", "light"};
    }

    std::vector<std::string> GetOutputs() const override {
        return {"shadowMap"};
    }

    std::string GetName() const override {
        return m_name;
    }

private:
    std::string m_name;
};

} // namespace SmartRenderer
