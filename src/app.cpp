#include <drogular/app.hpp>

#include <drogon/drogon.h>

namespace drogular {

App::App() {
    services_.setOptions(&options_);
}

void App::run(unsigned short port) {
    for (const auto& mapping : options_.staticFiles()) {
        router_.staticFiles(
            mapping.routePrefix,
            mapping.directory
        );
    }

    drogon::app()
        .addListener("0.0.0.0", port)
        .run();
}

} // namespace drogular
