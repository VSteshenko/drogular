#pragma once

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

    /**
     * Registers a page instance for the given path.
     */
    void page(const std::string& path, std::shared_ptr<Page> page);
};

} // namespace drogular
