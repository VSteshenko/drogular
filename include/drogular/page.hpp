#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>

#include <optional>

namespace drogular {

/**
 * Base class for full pages.
 *
 * A page is a component that can be mounted to a route.
 */
class Page : public Component {
public:
    ~Page() override = default;

    /**
     * Returns the GraphQL query required by this page.
     */
    virtual std::optional<gql::Query> query() const;
};

} // namespace drogular
