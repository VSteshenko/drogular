#pragma once

#include <drogular/diagnostics.hpp>
#include <drogular/services.hpp>

#include <json/json.h>

#include <functional>
#include <memory>
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

struct InspectionSection {
    std::string id;
    std::string title;
    std::string component;
    Json::Value data{Json::objectValue};
};

struct ApplicationInspection {
    static constexpr int SchemaVersion = 3;

    std::vector<RouteInspection> routes;
    std::vector<ComponentInspection> components;
    std::vector<ServiceInspection> services;
    std::vector<Diagnostic> diagnostics;
    std::vector<InspectionSection> sections;

    void addSection(InspectionSection section);
};

class DeveloperToolsContributor {
public:
    virtual ~DeveloperToolsContributor() = default;
    virtual void contribute(ApplicationInspection& inspection) const = 0;
};

class DeveloperToolsContributors {
public:
    void add(std::shared_ptr<DeveloperToolsContributor> contributor);
    const std::vector<std::shared_ptr<DeveloperToolsContributor>>& entries() const;
    void contribute(ApplicationInspection& inspection) const;

private:
    std::vector<std::shared_ptr<DeveloperToolsContributor>> contributors_;
};

// Compatibility aliases for the initial inspection API names.
using InspectionContributor = DeveloperToolsContributor;
using InspectionContributors = DeveloperToolsContributors;

using ApplicationInspectionProvider =
    std::function<ApplicationInspection()>;

const char* toString(RouteKind kind);
const char* toString(ServiceLifetime lifetime);
const char* toString(DiagnosticSeverity severity);

Json::Value toJson(const ApplicationInspection& inspection);

} // namespace drogular