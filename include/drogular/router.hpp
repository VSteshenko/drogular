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

private:
    ApplicationServices* services_ = nullptr;
};

} // namespace drogular
