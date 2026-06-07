#pragma once

#include <drogular/component.hpp>

namespace drogular {

/**
 * Base class for full pages.
 *
 * A page is a component that can be mounted to a route.
 */
class Page : public Component {
public:
    ~Page() override = default;
};

} // namespace drogular
