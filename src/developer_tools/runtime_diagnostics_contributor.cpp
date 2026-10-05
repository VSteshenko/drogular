#include <drogular/developer_tools/runtime_diagnostics_contributor.hpp>

#include <json/json.h>

#include <utility>

namespace drogular {
namespace {

Json::Value runtimeSection(
    const RuntimeDiagnosticsSnapshot& snapshot
) {
    Json::Value requests(Json::objectValue);
    requests["requests"] = Json::UInt64(snapshot.requests.requests);
    requests["actions"] = Json::UInt64(snapshot.requests.actions);
    requests["renderedActions"] =
        Json::UInt64(snapshot.requests.renderedActions);
    requests["redirects"] = Json::UInt64(snapshot.requests.redirects);

    Json::Value rendering(Json::objectValue);
    rendering["componentsRendered"] =
        Json::UInt64(snapshot.rendering.componentsRendered);
    rendering["renderFailures"] =
        Json::UInt64(snapshot.rendering.renderFailures);
    rendering["maximumDepth"] =
        Json::UInt64(snapshot.rendering.maximumDepth);

    Json::Value data(Json::objectValue);
    data["requests"] = std::move(requests);
    data["rendering"] = std::move(rendering);
    return data;
}

Json::Value interactionsSection(
    const RuntimeDiagnosticsSnapshot& snapshot
) {
    Json::Value data(Json::objectValue);
    data["requests"] = Json::UInt64(snapshot.interactions.requests);
    data["getRequests"] = Json::UInt64(snapshot.interactions.getRequests);
    data["postRequests"] = Json::UInt64(snapshot.interactions.postRequests);
    data["successfulResponses"] =
        Json::UInt64(snapshot.interactions.successfulResponses);
    data["clientErrorResponses"] =
        Json::UInt64(snapshot.interactions.clientErrorResponses);
    data["redirects"] = Json::UInt64(snapshot.interactions.redirects);
    return data;
}

Json::Value serviceScopesSection(
    const RuntimeDiagnosticsSnapshot& snapshot
) {
    Json::Value data(Json::objectValue);
    data["resolutions"] =
        Json::UInt64(snapshot.serviceScopes.resolutions);
    data["cacheHits"] =
        Json::UInt64(snapshot.serviceScopes.cacheHits);
    data["servicesCreated"] =
        Json::UInt64(snapshot.serviceScopes.servicesCreated);
    return data;
}

} // namespace

RuntimeDiagnosticsContributor::RuntimeDiagnosticsContributor(
    const RuntimeDiagnostics& diagnostics
) noexcept
    : diagnostics_(diagnostics) {
}

void RuntimeDiagnosticsContributor::contribute(
    ApplicationInspection& inspection
) const {
    const auto snapshot = diagnostics_.snapshot();

    inspection.addSection({
        "runtime",
        "Runtime",
        "drogular.runtime",
        runtimeSection(snapshot)
    });
    inspection.addSection({
        "interactions",
        "Interactions",
        "drogular.interactions",
        interactionsSection(snapshot)
    });
    inspection.addSection({
        "service-scopes",
        "Service Scopes",
        "drogular.service-scopes",
        serviceScopesSection(snapshot)
    });
}

} // namespace drogular