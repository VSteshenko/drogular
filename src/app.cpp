#include <drogular/app.hpp>
#include <drogular/developer_tools/diagnostics_page.hpp>
#include <drogular/developer_tools/diagnostics_resources.hpp>

#include <drogon/drogon.h>

#include <stdexcept>

namespace drogular {

App::App() {
    services_.setOptions(&options_);
}

App& App::profile(ApplicationProfile profile) {
    if (developerToolsEnabled_ && profile != ApplicationProfile::Development) {
        throw std::logic_error(
            "Developer Tools are already registered and cannot be disabled by changing the application profile"
        );
    }

    profile_ = profile;

    if (profile_ == ApplicationProfile::Development &&
        developerToolsOverride_.value_or(true)) {
        enableDeveloperTools();
    }

    return *this;
}

App& App::enableDeveloperTools() {
    developerToolsOverride_ = true;

    if (developerToolsEnabled_) {
        return *this;
    }

    enableDiagnosticsPage();
    developerToolsEnabled_ = true;

    return *this;
}

App& App::disableDeveloperTools() {
    if (developerToolsEnabled_ || inspectionEnabled_ || diagnosticsPageEnabled_) {
        throw std::logic_error(
            "Developer Tools are already registered and cannot be disabled"
        );
    }

    developerToolsOverride_ = false;

    return *this;
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

    const auto& componentDiagnostics =
        services_.components().diagnostics().entries();
    result.diagnostics.insert(
        result.diagnostics.end(),
        componentDiagnostics.begin(),
        componentDiagnostics.end()
    );

    if (inspectionEnabled_) {
        result.routes.push_back({
            ApplicationInspectionController::Path,
            RouteKind::Inspection,
            "GET",
            "ApplicationInspectionController"
        });
    }

    if (diagnosticsPageEnabled_) {
        result.routes.push_back({
            DeveloperToolsComponentRegistry::Path,
            RouteKind::Inspection,
            "GET",
            "DeveloperToolsComponentRegistry"
        });
    }

    const auto contributors =
        services_.service<DeveloperToolsContributors>();
    if (contributors != nullptr) {
        contributors->contribute(result);
    }

    return result;
}

App& App::enableInspection() {
    if (inspectionEnabled_) {
        return *this;
    }

    if (!services_.hasService(
            std::type_index(typeid(DeveloperToolsContributors))
    )) {
        services_.registerService<DeveloperToolsContributors>(
            std::make_shared<DeveloperToolsContributors>()
        );
    }

    if (!services_.hasService(
            std::type_index(typeid(ApplicationInspectionProvider))
        )) {
        services_.registerService<ApplicationInspectionProvider>(
            std::make_shared<ApplicationInspectionProvider>(
                [this] {
                    return inspect();
                }
            )
        );
    }

    inspectionController_ =
        std::make_shared<ApplicationInspectionController>(&services_);

    auto controller = inspectionController_;
    drogon::app().registerHandler(
        ApplicationInspectionController::Path,
        [controller](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            controller->handle(request, std::move(callback));
        },
        {drogon::Get}
    );

    inspectionEnabled_ = true;
    return *this;
}

App& App::enableDiagnosticsPage() {
    if (diagnosticsPageEnabled_) {
        return *this;
    }

    enableInspection();

    auto componentRegistry =
        services_.service<DeveloperToolsComponentRegistry>();
    if (componentRegistry == nullptr) {
        componentRegistry =
            std::make_shared<DeveloperToolsComponentRegistry>();
        services_.registerService<DeveloperToolsComponentRegistry>(
            componentRegistry
        );
    }

    router_.page(
        DiagnosticsPage::Path,
        [] {
            return std::make_shared<DiagnosticsPage>();
        },
        typeid(DiagnosticsPage).name()
    );

    drogon::app().registerHandler(
        DeveloperToolsComponentRegistry::Path,
        [componentRegistry](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            callback(
                drogon::HttpResponse::newHttpJsonResponse(
                    componentRegistry->toJson()
                )
            );
        },
        {drogon::Get}
    );

    drogon::app().registerHandler(
        std::string(DiagnosticsPage::AssetsPath) + "/diagnostics.css",
        [](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            auto response = drogon::HttpResponse::newHttpResponse();
            response->addHeader("Content-Type", "text/css; charset=utf-8");
            response->setBody(
                std::string(diagnostics_resources::stylesheet())
            );

            callback(response);
        },
        {drogon::Get}
    );

    drogon::app().registerHandler(
        std::string(DiagnosticsPage::AssetsPath) + "/diagnostics.js",
        [](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            auto response = drogon::HttpResponse::newHttpResponse();
            response->addHeader("Content-Type", "text/javascript; charset=utf-8");
            response->setBody(
                std::string(diagnostics_resources::script())
            );

            callback(response);
        },
        {drogon::Get}
    );

    diagnosticsPageEnabled_ = true;
    developerToolsEnabled_ = true;

    return *this;
}

App& App::developerToolsContributor(
    std::shared_ptr<DeveloperToolsContributor> contributor
) {
    auto contributors =
        services_.service<DeveloperToolsContributors>();
    if (contributors == nullptr) {
        contributors = std::make_shared<DeveloperToolsContributors>();
        services_.registerService<DeveloperToolsContributors>(contributors);
    }

    contributors->add(std::move(contributor));

    return *this;
}

App& App::developerToolsComponent(
    std::string name,
    std::string module
) {
    auto registry =
        services_.service<DeveloperToolsComponentRegistry>();
    if (registry == nullptr) {
        registry = std::make_shared<DeveloperToolsComponentRegistry>();
        services_.registerService<DeveloperToolsComponentRegistry>(registry);
    }

    registry->add(std::move(name), std::move(module));

    return *this;
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
            offlinePageFactory_,
            "offline-page"
        );
    }

    drogon::app()
        .addListener("0.0.0.0", port)
        .run();
}

} // namespace drogular