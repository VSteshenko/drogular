#include <drogular/application_inspection.hpp>

#include <stdexcept>
#include <utility>

namespace drogular {

void ApplicationInspection::addSection(InspectionSection section) {
    if (section.id.empty()) {
        throw std::invalid_argument("Inspection section id must not be empty");
    }

    for (auto& existing : sections) {
        if (existing.id == section.id) {
            existing = std::move(section);
            return;
        }
    }

    sections.push_back(std::move(section));
}

void DeveloperToolsContributors::add(
    std::shared_ptr<DeveloperToolsContributor> contributor
) {
    if (contributor == nullptr) {
        throw std::invalid_argument("Developer Tools contributor must not be null");
    }

    contributors_.push_back(std::move(contributor));
}

const std::vector<std::shared_ptr<DeveloperToolsContributor>>& DeveloperToolsContributors::entries() const {
    return contributors_;
}

void DeveloperToolsContributors::contribute(
    ApplicationInspection& inspection
) const {
    for (const auto& contributor : contributors_) {
        contributor->contribute(inspection);
    }
}

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

namespace {

Json::Value routesJson(const std::vector<RouteInspection>& values) {
    Json::Value result(Json::arrayValue);
    for (const auto& route : values) {
        Json::Value item(Json::objectValue);

        item["path"] = route.path;
        item["kind"] = toString(route.kind);
        item["method"] = route.method;
        item["target"] = route.target;

        result.append(std::move(item));
    }

    return result;
}

Json::Value componentsJson(const std::vector<ComponentInspection>& values) {
    Json::Value result(Json::arrayValue);
    for (const auto& component : values) {
        Json::Value item(Json::objectValue);

        item["tag"] = component.tag;

        result.append(std::move(item));
    }

    return result;
}

Json::Value servicesJson(const std::vector<ServiceInspection>& values) {
    Json::Value result(Json::arrayValue);
    for (const auto& service : values) {
        Json::Value item(Json::objectValue);

        item["type"] = service.type;
        item["lifetime"] = toString(service.lifetime);
        item["instantiated"] = service.instantiated;

        result.append(std::move(item));
    }

    return result;
}

Json::Value diagnosticsJson(const std::vector<Diagnostic>& values) {
    Json::Value result(Json::arrayValue);
    for (const auto& diagnostic : values) {
        Json::Value item(Json::objectValue);

        item["code"] = diagnostic.code;
        item["severity"] = toString(diagnostic.severity);
        item["message"] = diagnostic.message;
        item["location"]["source"] = diagnostic.location.source;
        item["location"]["position"] = Json::UInt64(diagnostic.location.position);
        item["location"]["line"] = Json::UInt64(diagnostic.location.line);
        item["location"]["column"] = Json::UInt64(diagnostic.location.column);

        result.append(std::move(item));
    }

    return result;
}

}

Json::Value toJson(const ApplicationInspection& inspection) {
    Json::Value root(Json::objectValue);
    root["schemaVersion"] = ApplicationInspection::SchemaVersion;

    root["routes"] = routesJson(inspection.routes);
    root["components"] = componentsJson(inspection.components);
    root["services"] = servicesJson(inspection.services);
    root["diagnostics"] = diagnosticsJson(inspection.diagnostics);

    Json::Value sections(Json::arrayValue);

    auto appendSection = [&sections](
        const char* id,
        const char* title,
        const char* component,
        Json::Value data
    ) {
        Json::Value section(Json::objectValue);

        section["id"] = id;
        section["title"] = title;
        section["component"] = component;
        section["data"] = std::move(data);

        sections.append(std::move(section));
    };

    appendSection(
        "routes",
        "Routes",
        "drogular.routes",
        routesJson(inspection.routes)
    );
    appendSection(
        "components",
        "Components",
        "drogular.components",
        componentsJson(inspection.components)
    );
    appendSection(
        "services",
        "Services",
        "drogular.services",
        servicesJson(inspection.services)
    );
    appendSection(
        "diagnostics",
        "Diagnostics",
        "drogular.diagnostics",
        diagnosticsJson(inspection.diagnostics)
    );

    for (const auto& custom : inspection.sections) {
        Json::Value section(Json::objectValue);

        section["id"] = custom.id;
        section["title"] = custom.title.empty() ? custom.id : custom.title;
        section["component"] = custom.component;
        section["data"] = custom.data;

        sections.append(std::move(section));
    }

    root["sections"] = std::move(sections);
    return root;
}

} // namespace drogular