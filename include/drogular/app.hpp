#pragma once

#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/router.hpp>
#include <drogular/action_handler.hpp>

#include <memory>
#include <string>
#include <type_traits>

namespace drogular {

/**
 * Main application entry point.
 *
 * App wraps Drogon and provides the high-level Drogular API.
 */
class App {
public:
    App() = default;

    /**
     * Sets the GraphQL client used by application pages.
     */
    App& graphQLClient(std::shared_ptr<GraphQLClient> client) {
        services_.setGraphQLClient(std::move(client));
        return *this;
    }

    /**
     * Registers a page type for the given path.
     */
    template <typename PageType>
    App& page(const std::string& path) {
        static_assert(
            std::is_base_of_v<Page, PageType>,
            "PageType must inherit from drogular::Page"
        );

        router_.page(path, std::make_shared<PageType>());
        return *this;
    }

    /**
     * Registers a component type by tag name.
     */
    template <typename ComponentType>
    App& component(const std::string& tag) {
        services_.components().registerComponent<ComponentType>(tag);
        return *this;
    }

    /**
     * Registers a component type using ComponentType::tag.
     */
    template <typename ComponentType>
    App& component() {
        services_.components().registerComponent<ComponentType>();
        return *this;
    }

    /**
     * Registers an action type for the given POST path.
     */
    template <typename ActionType>
    App& action(const std::string& path) {
        static_assert(
            std::is_base_of_v<ActionHandler, ActionType>,
            "ActionType must inherit from drogular::ActionHandler"
        );

        router_.action(
            path,
            std::make_shared<ActionType>()
        );

        return *this;
    }

    /**
     * Starts the HTTP server on the given port.
     */
    void run(unsigned short port);

private:
    ApplicationServices services_;
    Router router_{&services_};
};

} // namespace drogular
