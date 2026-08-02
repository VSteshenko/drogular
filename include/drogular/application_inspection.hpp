#pragma once

#include <drogular/services.hpp>

#include <string>
#include <vector>

namespace drogular {

enum class RouteKind {
    Page,
    Action,
    StaticFiles,
    ServiceWorker,
    OfflinePage
};

struct RouteInspection {
    std::string path;
    RouteKind kind = RouteKind::Page;
    std::string method;
    std::string target;
};

struct ComponentInspection {
    std::string tag;
};

using ServiceInspection = ServiceRegistration;

struct ApplicationInspection {
    std::vector<RouteInspection> routes;
    std::vector<ComponentInspection> components;
    std::vector<ServiceInspection> services;
};

const char* toString(RouteKind kind);
const char* toString(ServiceLifetime lifetime);

} // namespace drogular