#include <drogular/application_inspection.hpp>

namespace drogular {

const char* toString(RouteKind kind) {
    switch (kind) {
    case RouteKind::Page:
        return "page";

    case RouteKind::Action:
        return "action";

    case RouteKind::StaticFiles:
        return "static-files";

    case RouteKind::ServiceWorker:
        return "service-worker";

    case RouteKind::OfflinePage:
        return "offline-page";
    }

    return "unknown";
}
const char* toString(ServiceLifetime lifetime) {
    switch (lifetime) {
    case ServiceLifetime::Singleton:
        return "singleton";

    case ServiceLifetime::LazySingleton:
        return "lazy-singleton";

    case ServiceLifetime::Transient:
        return "transient";

    case ServiceLifetime::Scoped:
        return "scoped";
    }

    return "unknown";
}

} // namespace drogular