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

Json::Value jobsSection(const RuntimeDiagnosticsSnapshot& snapshot) {
    Json::Value data(Json::objectValue);
#define JOB_FIELD(field) data[#field] = Json::UInt64(snapshot.jobs.field)
    JOB_FIELD(submitted);
    JOB_FIELD(started);
    JOB_FIELD(completed);
    JOB_FIELD(failed);
    JOB_FIELD(cancelled);
    JOB_FIELD(cancellationRequests);
    JOB_FIELD(progressUpdates);
    JOB_FIELD(rejected);
    JOB_FIELD(queued);
    JOB_FIELD(running);
    JOB_FIELD(workerCapacity);
    JOB_FIELD(queueCapacity);
    JOB_FIELD(peakQueued);
    JOB_FIELD(peakRunning);
    JOB_FIELD(queueWaitMaxMs);
    JOB_FIELD(executionMaxMs);
    JOB_FIELD(queueWaitSamples);
    JOB_FIELD(executionSamples);
    JOB_FIELD(queueWaitAverageMs);
    JOB_FIELD(executionAverageMs);
    JOB_FIELD(workerUtilizationBps);
    JOB_FIELD(queueSaturationBps);
    JOB_FIELD(queueWaitTotalMs);
    JOB_FIELD(executionTotalMs);
    JOB_FIELD(eventsPublished);
    JOB_FIELD(eventGaps);
    JOB_FIELD(activeSubscriptions);
    JOB_FIELD(activeSseConnections);
    JOB_FIELD(sseConnectionsOpened);
    JOB_FIELD(sseConnectionsRejected);
    JOB_FIELD(sseConnectionsClosed);
#undef JOB_FIELD
    return data;
}

} // namespace

RuntimeDiagnosticsContributor::RuntimeDiagnosticsContributor(
    const RuntimeDiagnostics& diagnostics,
    const ProcessMetricsProvider* processMetrics,
    const ProcessMetricsHistory* history
) noexcept
    : diagnostics_(diagnostics), processMetrics_(processMetrics), history_(history)
{
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
    if (processMetrics_ != nullptr) {
        // Do not disturb the CPU sampling interval with extra inspection reads.
        const auto recorded = history_
            ? history_->snapshot()
            : std::vector<ProcessMetricsHistorySample>{};
        const auto metrics = recorded.empty()
            ? processMetrics_->snapshot()
            : recorded.back().metrics;
        Json::Value data(Json::objectValue);
        data["available"] = metrics.available;
        data["cpuPercent"] = metrics.cpuPercent ? Json::Value(*metrics.cpuPercent) : Json::Value();
        data["residentBytes"] = Json::UInt64(metrics.residentBytes);
        data["peakResidentBytes"] = Json::UInt64(metrics.peakResidentBytes);
        data["threadCount"] = Json::UInt64(metrics.threadCount);
        data["uptimeMs"] = Json::UInt64(metrics.uptimeMs);
        data["cpuTimeMs"] = Json::UInt64(metrics.cpuTimeMs);
        if (history_ != nullptr) {
            Json::Value samples(Json::arrayValue);
            for (const auto& point : recorded) {
                Json::Value sample(Json::objectValue);
                sample["elapsedMs"] = Json::UInt64(point.elapsedMs);
                sample["cpuPercent"] = point.metrics.cpuPercent
                    ? Json::Value(*point.metrics.cpuPercent)
                    : Json::Value();
                sample["residentBytes"] = Json::UInt64(point.metrics.residentBytes);
                samples.append(std::move(sample));
            }
            data["history"] = std::move(samples);
            data["samplingIntervalMs"] = 2000;
        }
        inspection.addSection({
            "process",
            "Process",
            "drogular.process",
            std::move(data)
        });
    }
    inspection.addSection({
        "interactions",
        "Interactions",
        "drogular.interactions",
        interactionsSection(snapshot)
    });
    inspection.addSection({
        "jobs",
        "Jobs",
        "drogular.jobs",
        jobsSection(snapshot)
    });
    inspection.addSection({
        "service-scopes",
        "Service Scopes",
        "drogular.service-scopes",
        serviceScopesSection(snapshot)
    });
}

} // namespace drogular