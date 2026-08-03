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
    Json::Value data{Json::objectValue};
};

struct ApplicationInspection {
    static constexpr int SchemaVersion = 2;

    std::vector<RouteInspection> routes;
    std::vector<ComponentInspection> components;
    std::vector<ServiceInspection> services;
    std::vector<Diagnostic> diagnostics;
    std::vector<InspectionSection> sections;

    void addSection(InspectionSection section);
};

class InspectionContributor {
public:
    virtual ~InspectionContributor() = default;
    virtual void contribute(ApplicationInspection& inspection) const = 0;
};

class InspectionContributors {
public:
    void add(std::shared_ptr<InspectionContributor> contributor);
    const std::vector<std::shared_ptr<InspectionContributor>>& entries() const;
    void contribute(ApplicationInspection& inspection) const;

private:
    std::vector<std::shared_ptr<InspectionContributor>> contributors_;
};

using ApplicationInspectionProvider =
    std::function<ApplicationInspection()>;

const char* toString(RouteKind kind);
const char* toString(ServiceLifetime lifetime);
const char* toString(DiagnosticSeverity severity);

Json::Value toJson(const ApplicationInspection& inspection);

} // namespace drogular