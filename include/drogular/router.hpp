#pragma once

#include <drogular/services.hpp>
#include <drogular/action_handler.hpp>

#include <memory>
#include <string>

namespace drogular {

class Page;

/**
 * Router connects Drogular pages to Drogon routes.
 */
class Router {
public:
    Router() = default;
    explicit Router(ApplicationServices* services = nullptr);

    /**
     * Registers a page instance for the given path.
     */
    void page(const std::string& path, std::shared_ptr<Page> page);

    /**
     * Registers an action instance for the given path.
     */
    void action(const std::string& path, std::shared_ptr<ActionHandler> action);

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

private:
    ApplicationServices* services_ = nullptr;
};

} // namespace drogular