#include "jobs_demo_routes.hpp"

#include <drogular/job_runtime.hpp>
#include <drogular/job_sse_drogon.hpp>
#include <drogular/runtime_diagnostics.hpp>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>

#include <charconv>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <functional>
#include <exception>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace {

using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

Json::Value jobJson(const drogular::JobSnapshot& job) {
    Json::Value value(Json::objectValue);
    value["id"] = Json::UInt64(job.id);
    value["name"] = job.name;
    switch (job.status) {
    case drogular::JobStatus::Queued:
        value["status"] = "Queued";
        break;
    case drogular::JobStatus::Running:
        value["status"] = "Running";
        break;
    case drogular::JobStatus::Completed:
        value["status"] = "Completed";
        break;
    case drogular::JobStatus::Failed:
        value["status"] = "Failed";
        break;
    case drogular::JobStatus::Cancelled:
        value["status"] = "Cancelled";
        break;
    }
    value["progress"] = job.progress;
    value["message"] = job.message;
    value["cancellationRequested"] = job.cancellationRequested;
    if (job.failure) {
        value["error"] = job.failure->message;
        value["errorCode"] = job.failure->code;
    } else if (!job.error.empty()) {
        value["error"] = job.error;
    }
    if (job.queueDuration) {
        value["queueMs"] = Json::Int64(job.queueDuration->count());
    }
    if (job.executionDuration) {
        value["executionMs"] = Json::Int64(job.executionDuration->count());
    }
    if (const auto* result = job.resultAs<std::string>()) {
        value["result"] = *result;
    }

    return value;
}

drogon::HttpResponsePtr jsonResponse(
    Json::Value value,
    drogon::HttpStatusCode status = drogon::k200OK
) {
    auto response = drogon::HttpResponse::newHttpJsonResponse(std::move(value));
    response->setStatusCode(status);
    // Zero means cache forever in Drogon; JSON endpoints need fresh state.
    response->setExpiredTime(-1);
    response->addHeader("Cache-Control", "no-store");
    return response;
}

std::optional<drogular::JobId> parseId(std::string_view text) {
    drogular::JobId result{};
    if (text.empty()) {
        return std::nullopt;
    }
    const auto [last, ec] =
        std::from_chars(text.data(), text.data() + text.size(), result);
    if (ec != std::errc{} || last != text.data() + text.size() || result == 0) {
        return std::nullopt;
    }
    return result;
}

Json::Value metricJson(const drogular::JobRuntimeDiagnostics& d) {
    Json::Value json(Json::objectValue);
#define JOB_METRIC(field) json[#field] = Json::UInt64(d.field)
    JOB_METRIC(queued);
    JOB_METRIC(running);
    JOB_METRIC(workerCapacity);
    JOB_METRIC(submitted);
    JOB_METRIC(started);
    JOB_METRIC(completed);
    JOB_METRIC(failed);
    JOB_METRIC(cancelled);
    JOB_METRIC(rejected);
    JOB_METRIC(cancellationRequests);
    JOB_METRIC(progressUpdates);
    JOB_METRIC(queueWaitTotalMs);
    JOB_METRIC(executionTotalMs);
    JOB_METRIC(eventsPublished);
    JOB_METRIC(eventGaps);
    JOB_METRIC(activeSubscriptions);
    JOB_METRIC(activeSseConnections);
    JOB_METRIC(sseConnectionsOpened);
    JOB_METRIC(sseConnectionsRejected);
    JOB_METRIC(sseConnectionsClosed);
#undef JOB_METRIC
    return json;
}

void installRoutesInternal(std::shared_ptr<drogular::JobRuntime> jobs,
                           drogular::RuntimeDiagnostics& diagnostics
) {
    drogon::app().registerHandler("/api/jobs",
        [jobs](const drogon::HttpRequestPtr&, ResponseCallback&& callback) {
            Json::Value array(Json::arrayValue);
            for (const auto& job : jobs->list()) {
                array.append(jobJson(job));
            }
            callback(jsonResponse(std::move(array)));
        }, {drogon::Get});

    drogon::app().registerHandler("/api/jobs/create",
        [jobs](const drogon::HttpRequestPtr& request, ResponseCallback&& callback) {
            // Demo controls are intentionally bounded, rather than accepting arbitrary tasks.
            const auto secondsParam = request->getParameter("seconds");
            int seconds = 0;
            const auto [end, ec] = std::from_chars(
                secondsParam.data(), secondsParam.data() + secondsParam.size(), seconds);
            if (ec != std::errc{} ||
                end != secondsParam.data() + secondsParam.size() ||
                seconds < 1 || seconds > 120) {
                Json::Value error;
                error["error"] = "seconds must be an integer between 1 and 120";
                callback(jsonResponse(std::move(error), drogon::k400BadRequest));
                return;
            }
            const bool fail = request->getParameter("fail") == "1";
            try {
                const auto id = jobs->submit(fail ? "Failure demonstration" : "Background operation",
                    [seconds, fail](drogular::JobContext& ctx) {
                        for (int step = 1; step <= seconds * 5; ++step) {
                            ctx.throwIfCancellationRequested();
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            ctx.throwIfCancellationRequested();
                            ctx.reportProgress(static_cast<unsigned>(step * 100 / (seconds * 5)),
                                               "Processing batch " + std::to_string(step) +
                                               " of " + std::to_string(seconds * 5));
                        }
                        if (fail) {
                            throw drogular::JobFailure("DEMO_FAILURE", "Intentional demo failure");
                        }
                        ctx.setResult(std::string("All batches processed successfully"));
                    });
                callback(jsonResponse(jobJson(*jobs->find(id)), drogon::k201Created));
            } catch (const std::exception& error) {
                Json::Value response;
                response["error"] = error.what();
                callback(jsonResponse(std::move(response), drogon::k503ServiceUnavailable));
            }
        }, {drogon::Post});

    drogon::app().registerHandler("/api/jobs/detail",
        [jobs](const drogon::HttpRequestPtr& request, ResponseCallback&& callback) {
            const auto id = parseId(request->getParameter("id"));
            if (!id) {
                Json::Value error; error["error"] = "Invalid job ID";
                callback(jsonResponse(std::move(error), drogon::k400BadRequest));
                return;
            }
            const auto job = jobs->find(*id);
            if (!job) {
                Json::Value error; error["error"] = "Unknown job";
                callback(jsonResponse(std::move(error), drogon::k404NotFound));
                return;
            }
            callback(jsonResponse(jobJson(*job)));
        }, {drogon::Get});

    drogon::app().registerHandler("/api/jobs/cancel",
        [jobs](const drogon::HttpRequestPtr& request, ResponseCallback&& callback) {
            const auto id = parseId(request->getParameter("id"));
            if (!id) {
                Json::Value error; error["error"] = "Invalid job ID";
                callback(jsonResponse(std::move(error), drogon::k400BadRequest));
                return;
            }
            if (!jobs->requestCancel(*id)) {
                Json::Value error; error["error"] = "Unknown job";
                callback(jsonResponse(std::move(error), drogon::k404NotFound));
                return;
            }
            callback(jsonResponse(jobJson(*jobs->find(*id))));
        }, {drogon::Post});

    drogon::app().registerHandler("/api/jobs/diagnostics",
        [&diagnostics](const drogon::HttpRequestPtr&, ResponseCallback&& callback) {
            callback(jsonResponse(metricJson(diagnostics.snapshot().jobs)));
        }, {drogon::Get});

    // Intentionally no auth: local demonstration only, never deploy publicly.
    drogular::registerJobSseEndpoint(drogon::app(), "/api/jobs/events", jobs,
        [](const drogon::HttpRequestPtr&, drogular::JobId) { return true; });
}

} // namespace

void jobsdemo::installRoutes(std::shared_ptr<drogular::JobRuntime> jobs,
                             drogular::RuntimeDiagnostics& diagnostics) {
    installRoutesInternal(std::move(jobs), diagnostics);
}