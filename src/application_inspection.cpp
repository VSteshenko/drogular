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

    case RouteKind::Inspection:
        return "inspection";
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

const char* toString(DiagnosticSeverity severity) {
    switch (severity) {
    case DiagnosticSeverity::Info:
        return "info";

    case DiagnosticSeverity::Warning:
        return "warning";

    case DiagnosticSeverity::Error:
        return "error";
    }

    return "unknown";
}

Json::Value toJson(const ApplicationInspection& inspection) {
    Json::Value root(Json::objectValue);
    root["schemaVersion"] =
        ApplicationInspection::SchemaVersion;

    Json::Value routes(Json::arrayValue);
    for (const auto& route : inspection.routes) {
        Json::Value item(Json::objectValue);

        item["path"] = route.path;
        item["kind"] = toString(route.kind);
        item["method"] = route.method;
        item["target"] = route.target;

        routes.append(std::move(item));
    }

    root["routes"] = std::move(routes);

    Json::Value components(Json::arrayValue);
    for (const auto& component : inspection.components) {
        Json::Value item(Json::objectValue);

        item["tag"] = component.tag;

        components.append(std::move(item));
    }

    root["components"] = std::move(components);

    Json::Value services(Json::arrayValue);
    for (const auto& service : inspection.services) {
        Json::Value item(Json::objectValue);

        item["type"] = service.type;
        item["lifetime"] = toString(service.lifetime);
        item["instantiated"] = service.instantiated;

        services.append(std::move(item));
    }

    root["services"] = std::move(services);

    Json::Value diagnostics(Json::arrayValue);
    for (const auto& diagnostic : inspection.diagnostics) {
        Json::Value item(Json::objectValue);

        item["code"] = diagnostic.code;
        item["severity"] = toString(diagnostic.severity);
        item["message"] = diagnostic.message;
        item["location"]["source"] = diagnostic.location.source;
        item["location"]["position"] = Json::UInt64(diagnostic.location.position);
        item["location"]["line"] = Json::UInt64(diagnostic.location.line);
        item["location"]["column"] = Json::UInt64(diagnostic.location.column);
        diagnostics.append(std::move(item));
    }

    root["diagnostics"] = std::move(diagnostics);

    return root;
}

} // namespace drogular