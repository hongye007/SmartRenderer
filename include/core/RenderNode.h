#pragma once

#include <string>
#include <vector>

namespace SmartRenderer {

class RenderContext;

class RenderNode {
public:
    virtual ~RenderNode() = default;
    virtual void Execute(RenderContext& context) = 0;
    virtual std::vector<std::string> GetInputs() const = 0;
    virtual std::vector<std::string> GetOutputs() const = 0;
    virtual std::string GetName() const = 0;
};

} // namespace SmartRenderer
