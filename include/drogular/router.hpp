#pragma once

#include <drogular/services.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/developer_tools/application_inspection.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace drogular {

class Page;

enum class ActionMethod {
    Get,
    Post
};

/**
 * Router connects Drogular pages to Drogon routes.
 */
class Router {
public:
    using PageFactory = std::function<std::shared_ptr<Page>()>;
    using ActionFactory = std::function<std::shared_ptr<ActionHandler>()>;

    Router() = default;
    explicit Router(ApplicationServices* services = nullptr);

    /**
     * Registers a page factory for the given path.
     * A fresh page instance is created for every request.
     */
    void page(
        const std::string& path,
        PageFactory factory,
        std::string target
    );

    /**
     * Registers an action factory for the given path.
     * A fresh action instance is created for every request.
     */
    void action(
        const std::string& path,
        ActionFactory factory,
        std::string target,
        ActionMethod method = ActionMethod::Post
    );

    /**
     * Registers a static file handler.
     *
     * Files requested under the specified route prefix
     * are resolved relative to the provided directory.
     *
     * Path traversal protection is applied separately.
     */
    void staticFiles(
        const std::string& routePrefix,
        const std::filesystem::path& directory
    );

    /**
     * Registers a root-level service worker route.
     *
     * The service worker is served from /service-worker.js,
     * regardless of where the source file is located on disk.
     */
    void serviceWorker(
        const std::filesystem::path& path
    );

    const std::vector<RouteInspection>& routes() const { return routes_; }

private:
    ApplicationServices* services_ = nullptr;
    std::vector<RouteInspection> routes_;
};

} // namespace drogular