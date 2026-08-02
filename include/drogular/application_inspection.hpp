#pragma once

#include <drogular/diagnostics.hpp>
#include <drogular/services.hpp>

#include <json/json.h>

#include <functional>
#include <string>
#include <vector>

namespace drogular {

enum class RouteKind {
    Page,
    Action,
    StaticFiles,
    ServiceWorker,
    OfflinePage,
    Inspection
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
    static constexpr int SchemaVersion = 1;

    std::vector<RouteInspection> routes;
    std::vector<ComponentInspection> components;
    std::vector<ServiceInspection> services;
    std::vector<Diagnostic> diagnostics;
};

using ApplicationInspectionProvider =
    std::function<ApplicationInspection()>;

const char* toString(RouteKind kind);
const char* toString(ServiceLifetime lifetime);
const char* toString(DiagnosticSeverity severity);

Json::Value toJson(const ApplicationInspection& inspection);

} // namespace drogular