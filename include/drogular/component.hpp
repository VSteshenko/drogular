#pragma once

#include <string>

namespace drogular {

/**
 * Provides data and services needed during rendering.
 */
class RenderContext {
public:
    RenderContext() = default;
};

/**
 * Base class for all UI components.
 *
 * Components are responsible for rendering reusable pieces of HTML.
 */
class Component {
public:
    virtual ~Component() = default;

    /**
     * Called before rendering.
     */
    virtual void onInit(RenderContext& context);

    /**
     * Renders the component into HTML.
     */
    virtual std::string render(RenderContext& context) = 0;
};

} // namespace drogular
