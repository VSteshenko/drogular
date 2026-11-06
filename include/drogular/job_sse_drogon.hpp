#pragma once

#include <drogular/job_runtime.hpp>

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace drogular {

struct JobSseOptions {
    std::chrono::milliseconds heartbeat{std::chrono::seconds(15)};
    std::size_t maxConnections = 64;
};

// Authorization MUST verify the requester is allowed to inspect this Job.
using JobSseAuthorize = std::function<bool(const drogon::HttpRequestPtr&, JobId)>;

/** Registers GET /.../events?job=<id> for authorized Job EventSource clients.
 * Owns a shared JobRuntime for the lifetime of active streams. The endpoint
 * accepts Last-Event-ID for bounded replay and sends job.reset on gaps.
 * No Drogon I/O thread blocks while waiting for events.
 */
void registerJobSseEndpoint(drogon::HttpAppFramework& app,
                            std::string path,
                            std::shared_ptr<JobRuntime> runtime,
                            JobSseAuthorize authorize,
                            JobSseOptions options = {});

} // namespace drogular