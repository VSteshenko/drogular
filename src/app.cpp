#include <drogular/app.hpp>

#include <drogon/drogon.h>

namespace drogular {

App::App() {
    services_.setOptions(&options_);
}

ApplicationInspection App::inspect() const {
    ApplicationInspection result;
    result.routes = router_.routes();

    for (const auto& mapping : options_.staticFiles()) {
        result.routes.push_back({
            mapping.routePrefix,
            RouteKind::StaticFiles,
            "GET",
            mapping.directory.string()
        });
    }

    if (options_.serviceWorkerPath().has_value()) {
        result.routes.push_back({
            "/service-worker.js",
            RouteKind::ServiceWorker,
            "GET",
            options_.serviceWorkerPath()->string()
        });
    }

    if (offlinePageRoute_.has_value()) {
        result.routes.push_back({
            *offlinePageRoute_,
            RouteKind::OfflinePage,
            "GET",
            {}
        });
    }

    for (const auto& tag : services_.components().tags()) {
        result.components.push_back({
            tag
        });
    }

    result.services =
        services_.registrations();

    return result;
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
