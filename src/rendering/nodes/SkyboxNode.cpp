#include "core/RenderNode.h"
#include "core/RenderContext.h"
#include <vector>
#include <string>

namespace SmartRenderer {

class SkyboxNode : public RenderNode {
public:
    SkyboxNode(const std::string& name) : m_name(name) {}

    void Execute(RenderContext& context) override {
        Renderer* renderer = context.GetRenderer();
        if (!renderer) return;

        // Render skybox
        // In real implementation, would render cube with skybox texture
    }

    std::vector<std::string> GetInputs() const override {
        return {"skyboxTexture", "camera"};
    }

    std::vector<std::string> GetOutputs() const override {
        return {"colorBuffer"};
    }

    std::string GetName() const override {
        return m_name;
    }

private:
    std::string m_name;
};

} // namespace SmartRenderer
