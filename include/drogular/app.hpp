#pragma once

#include <drogular/page.hpp>
#include <drogular/router.hpp>

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
     * Starts the HTTP server on the given port.
     */
    void run(unsigned short port);

private:
    Router router_;
};

} // namespace drogular
