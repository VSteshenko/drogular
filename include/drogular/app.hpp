#pragma once

#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/router.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/application_options.hpp>

#include <memory>
#include <string>
#include <type_traits>
#include <filesystem>
#include <utility>

namespace drogular {

/**
 * Main application entry point.
 *
 * App wraps Drogon and provides the high-level Drogular API.
 */
class App {
public:
    App();

    ApplicationOptions& options() {
        return options_;
    }

    const ApplicationOptions& options() const {
        return options_;
    }

    App& templateRoot(
        std::filesystem::path root
    ) {
        options_.setTemplateRoot(
            std::move(root)
        );

        return *this;
    }

    App& templateCache(bool enabled) {
        options_.setTemplateCacheEnabled(enabled);
        return *this;
    }

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

    /// Registers a static file mapping.
    ///
    /// Requests matching the specified route prefix are served
    /// from the given directory.
    ///
    /// Example:
    ///
    /// app.staticFiles(
    ///     "/assets",
    ///     "public"
    /// );
    ///
    /// Request:
    ///
    /// /assets/logo.png
    ///
    /// File:
    ///
    /// public/logo.png
    ///
    /// Static file mappings are typically used for:
    ///
    /// - Images
    /// - CSS
    /// - JavaScript
    /// - Favicons
    /// - Web manifests
    /// - Service workers
    App& staticFiles(
        std::string routePrefix,
        std::filesystem::path directory
    ) {
        options_.addStaticFiles(
            std::move(routePrefix),
            std::move(directory)
        );

        return *this;
    }

    /**
     * Enables or disables static file browser cache headers.
     */
    App& staticFileCache(bool enabled) {
        options_.setStaticFileCacheEnabled(enabled);
        return *this;
    }

    /**
     * Sets max-age for static file browser cache headers.
     */
    App& staticFileCacheMaxAge(
        std::chrono::seconds maxAge
    ) {
        options_.setStaticFileCacheMaxAge(maxAge);
        return *this;
    }

    /**
     * Starts the HTTP server on the given port.
     */
    void run(unsigned short port);

    ApplicationServices& services() {
        return services_;
    }

private:
    ApplicationOptions options_;
    ApplicationServices services_;
    Router router_{&services_};
};

} // namespace drogular