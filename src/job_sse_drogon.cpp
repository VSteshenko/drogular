#include <drogular/job_sse_drogon.hpp>

#include <drogular/job_events.hpp>
#include <drogular/job_sse.hpp>

#include <drogon/HttpResponse.h>

#include <atomic>
#include <charconv>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

namespace drogular {
namespace {

template <typename T>
std::optional<T> parseUnsigned(std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }

    T parsed{};
    const auto [end, error] =
        std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (error != std::errc{} || end != value.data() + value.size()) {
        return std::nullopt;
    }

    return parsed;
}

void reply(drogon::HttpResponsePtr response,
           std::function<void(const drogon::HttpResponsePtr&)>& callback
) {
    response->setExpiredTime(0);
    callback(response);
}

drogon::HttpResponsePtr errorResponse(drogon::HttpStatusCode status, std::string message) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(status);
    response->setContentTypeString("text/plain; charset=utf-8");
    response->setBody(std::move(message));
    return response;
}

struct ConnectionCount {
    std::atomic<std::size_t> active{0};
    std::size_t limit;
    bool acquire() {
        auto previous = active.load();
        while (previous < limit) {
            if (active.compare_exchange_weak(previous, previous + 1)) {
                return true;
            }
        }

        return false;
    }
};

struct ConnectionLease {
    std::shared_ptr<ConnectionCount> count;

    ~ConnectionLease() {
        if (count) {
            --count->active;
        }
    }
};

void forwardStream(std::shared_ptr<JobRuntime> jobs,
                   drogon::ResponseStreamPtr stream,
                   JobEventStream::Subscription subscription,
                   std::chrono::milliseconds heartbeat
) {
    (void)jobs;
    try {
        // Initial comment flushes headers promptly; doubles as a connection check.
        bool connected = stream->send(encodeJobSseHeartbeat());
        while (connected) {
            auto batch = subscription.waitFor(heartbeat);
            if (batch.gap) {
                // Reset invalidates all preceding frames, including replay in this batch.
                connected = stream->send(encodeJobSseReset());
                // The browser must fetch a fresh authorized snapshot before trusting progress.
                continue;
            }
            if (batch.events.empty()) {
                connected = stream->send(encodeJobSseHeartbeat());
            } else {
                for (const auto& event : batch.events) {
                    if (!(connected = stream->send(encodeJobSse(event)))) {
                        break;
                    }
                }
            }
        }
    } catch (...) {
        // A disconnected client or stream error must not escape the worker.
    }
    stream->close();
}

} // namespace

void registerJobSseEndpoint(drogon::HttpAppFramework& app,
                            std::string path,
                            std::shared_ptr<JobRuntime> runtime,
                            JobSseAuthorize authorize,
                            JobSseOptions options
) {
    if (!runtime || !authorize || path.empty() ||
        options.heartbeat <= std::chrono::milliseconds::zero() ||
        !options.maxConnections
    ) {
        throw std::invalid_argument("Invalid Job SSE endpoint options");
    }

    auto connections = std::make_shared<ConnectionCount>();
    connections->limit = options.maxConnections;
    app.registerHandler(std::move(path),
        [runtime = std::move(runtime), authorize = std::move(authorize),
         connections, heartbeat = options.heartbeat](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback)
        {
            const auto id =
                parseUnsigned<JobId>(request->getParameter("job"));
            if (!id || *id == 0) {
                reply(errorResponse(drogon::k400BadRequest, "Invalid job ID"), callback);
                return;
            }
            if (!authorize(request, *id)) {
                reply(errorResponse(drogon::k403Forbidden, "Access denied"), callback);
                return;
            }
            if (!runtime->find(*id)) {
                reply(errorResponse(drogon::k404NotFound, "Unknown job"), callback);
                return;
            }
            std::optional<std::uint64_t> cursor;
            const auto header = request->getHeader("Last-Event-ID");
            if (!header.empty()) {
                cursor = parseUnsigned<std::uint64_t>(header);
                if (!cursor) {
                    reply(errorResponse(drogon::k400BadRequest, "Invalid Last-Event-ID"), callback);
                    return;
                }
            }
            if (!connections->acquire()) {
                reply(errorResponse(drogon::k503ServiceUnavailable, "Too many SSE connections"), callback);
                return;
            }
            // Subscribe before handing over the HTTP response, so events between
            // validation and the asynchronous callback are not lost.
            auto subscription = runtime->events().subscribe(*id, cursor);
            auto lease = std::make_shared<ConnectionLease>();
            lease->count = connections;
            auto response = drogon::HttpResponse::newAsyncStreamResponse(
                [runtime, lease, heartbeat,
                 subscription = std::make_shared<JobEventStream::Subscription>(
                     std::move(subscription))](drogon::ResponseStreamPtr stream) mutable
                {
                    if (!stream) return;
                    try {
                        std::thread([runtime, stream = std::move(stream),
                                     subscription = std::move(*subscription), heartbeat,
                                     lease]() mutable
                        {
                            forwardStream(std::move(runtime), std::move(stream),
                                          std::move(subscription), heartbeat);
                        }).detach();
                    } catch (...) {
                        // Thread construction failure closes the stream via RAII.
                    }
                }, true);
            response->setContentTypeString("text/event-stream; charset=utf-8");
            response->addHeader("Cache-Control", "no-cache, no-transform");
            response->addHeader("X-Accel-Buffering", "no");
            response->setExpiredTime(0);
            callback(response);
        }, {drogon::Get});
}

} // namespace drogular