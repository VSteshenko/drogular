#pragma once

#include <drogular/services.hpp>
#include <drogular/security/http_security_policy.hpp>
#include <drogular/runtime_diagnostics.hpp>
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
    explicit Router(
        ApplicationServices* services = nullptr,
        RuntimeDiagnostics* diagnostics = nullptr
    );

    /**
     * Registers a page factory for the given path.
     * A fresh page instance is created for every request.
     */
    void page(
        const std::string& path,
        PageFactory factory,
        std::string target,
        security::HttpSecurityPolicy policy = {}
    );

    /**
     * Registers an action factory for the given path.
     * A fresh action instance is created for every request.
     */
    void action(
        const std::string& path,
        ActionFactory factory,
        std::string target,
        ActionMethod method = ActionMethod::Post,
        security::HttpSecurityPolicy policy = {}
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
        const std::filesystem::path& directory,
        bool recursive = false
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

    // Call during application setup, before the server begins processing requests.
    // Existing routes remain public unless registered with an explicit policy.
    void authenticationPipeline(std::shared_ptr<const security::AuthenticationPipeline> pipeline) {
        securityConfig_->pipeline = std::move(pipeline);
    }

    const std::vector<RouteInspection>& routes() const { return routes_; }

private:
    std::shared_ptr<security::HttpSecurityConfiguration> securityConfig_ =
        std::make_shared<security::HttpSecurityConfiguration>();
    ApplicationServices* services_ = nullptr;
    RuntimeDiagnostics* diagnostics_ = nullptr;
    std::vector<RouteInspection> routes_;
};

} // namespace drogular