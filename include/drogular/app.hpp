#pragma once

#include <string>

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
     * Starts the HTTP server on the given port.
     */
    void run(unsigned short port);
};

} // namespace drogular
