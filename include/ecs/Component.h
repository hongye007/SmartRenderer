#pragma once

namespace SmartRenderer {

// Base class for all components
// Components are pure data structures with no behavior
class Component {
public:
    virtual ~Component() = default;
};

} // namespace SmartRenderer
