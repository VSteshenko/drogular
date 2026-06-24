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

    if (options_.serviceWorkerPath().has_value()) {
        router_.serviceWorker(
            *options_.serviceWorkerPath()
        );
    }

    if (offlinePageRoute_.has_value() &&
        offlinePageFactory_) {
        router_.page(
            *offlinePageRoute_,
            offlinePageFactory_()
        );
    }

    drogon::app()
        .addListener("0.0.0.0", port)
        .run();
}

} // namespace drogular
