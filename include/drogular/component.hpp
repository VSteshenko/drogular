#pragma once

#include <string>

namespace drogular {

/**
 * Base class for all UI components.
 *
 * Components are responsible for rendering reusable pieces of HTML.
 */
class Component {
public:
    virtual ~Component() = default;

    /**
     * Renders the component into HTML.
     */
    virtual std::string render() = 0;
};

} // namespace drogular
