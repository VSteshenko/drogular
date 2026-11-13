#include <drogular/job_sse_drogon.hpp>
#include <drogular/job_events.hpp>
#include <drogular/job_sse.hpp>
#include <drogular/runtime_diagnostics.hpp>

#include <drogon/HttpResponse.h>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <optional>
#include <mutex>
#include <vector>
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
    response->setExpiredTime(-1);
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
    RuntimeDiagnostics* diagnostics = nullptr;

    ~ConnectionLease() {
        if (count) {
            --count->active;
            if (diagnostics) {
                diagnostics->recordJobSseClosed();
            }
        }
    }
};

void forwardStream(std::shared_ptr<JobRuntime> jobs,
                   const std::atomic_bool& stopRequested,
                   JobId jobId,
                   drogon::ResponseStreamPtr stream,
                   JobEventStream::Subscription subscription,
                   std::chrono::milliseconds heartbeat
) {
    try {
        // Initial comment flushes headers promptly; doubles as a connection check.
        bool connected = stream->send(encodeJobSseHeartbeat());
        // A Job may finish between the HTTP status check and the stream
        // callback. In that case there might be no terminal event buffered.
        // The browser will obtain the final snapshot through its JSON API.
        const auto initial = jobs->find(jobId);
        if (!initial || isTerminalJobStatus(initial->status)) {
            connected = false;
        }
        auto lastHeartbeat = std::chrono::steady_clock::now();
        while (connected && !stopRequested && !jobs->isStopping()) {
            // Short waits allow session shutdown to join promptly even with a
            // long configured heartbeat. Only send heartbeats at the configured interval.
            const auto waitTime = std::min(heartbeat, std::chrono::milliseconds(250));
            auto batch = subscription.waitFor(waitTime);
            if (stopRequested || jobs->isStopping()) {
                break;
            }
            if (batch.gap) {
                // Reset invalidates all preceding frames, including replay in this batch.
                connected = stream->send(encodeJobSseReset());
                // The browser must fetch a fresh authorized snapshot before trusting progress.
                continue;
            }
            if (batch.events.empty()) {
                const auto now = std::chrono::steady_clock::now();
                if (now - lastHeartbeat >= heartbeat) {
                    connected = stream->send(encodeJobSseHeartbeat());
                    lastHeartbeat = now;
                }
            } else {
                lastHeartbeat = std::chrono::steady_clock::now();
                for (const auto& event : batch.events) {
                    if (!(connected = stream->send(encodeJobSse(event)))) {
                        break;
                    }
                    // A job-scoped subscription has no further useful events
                    // after its terminal state; release the connection promptly.
                    if (isTerminalJobEvent(event.kind)) {
                        connected = false;
                        break;
                    }
                }
            }
        }
    } catch (...) {
        // A disconnected client or stream error must not escape the worker.
    }
    // The worker owns both subscription and connection lease until exit.
    // Closing an already disconnected Drogon stream is safe; avoid leaking it.
    stream->close();
}

// Owns all SSE forwarding threads. The endpoint callback retains this manager
// and its destructor requests cancellation and joins every remaining worker.
// Workers deliberately do not capture the manager, preventing ownership cycles.
class StreamWorkers {
public:
    explicit StreamWorkers(std::size_t maxConnections) {
        workers_.reserve(maxConnections);
    }

    ~StreamWorkers() {
        std::lock_guard lock(mutex_);
        for (auto& worker : workers_) {
            worker.stop->store(true);
        }
        for (auto& worker : workers_) {
            if (worker.thread.joinable()) {
                worker.thread.join();
            }
        }
    }

    void start(std::shared_ptr<JobRuntime> runtime, JobId id,
               drogon::ResponseStreamPtr stream,
               JobEventStream::Subscription subscription,
               std::chrono::milliseconds heartbeat,
               std::shared_ptr<ConnectionLease> lease) {
        auto stop = std::make_shared<std::atomic_bool>(false);
        auto done = std::make_shared<std::atomic_bool>(false);
        std::lock_guard lock(mutex_);
        // Join completed workers before adding a new one, so long-running
        // servers do not accumulate thread handles after client disconnects.
        auto it = workers_.begin();
        while (it != workers_.end()) {
            if (it->done->load()) {
                if (it->thread.joinable()) {
                    it->thread.join();
                }
                it = workers_.erase(it);
            } else {
                ++it;
            }
        }
        Worker worker;
        worker.stop = std::move(stop);
        worker.done = std::move(done);
        worker.thread = std::thread(
            [runtime = std::move(runtime), id, stream = std::move(stream),
             subscription = std::move(subscription), heartbeat,
             lease = std::move(lease), stop = worker.stop,
             done = worker.done]() mutable {
                try {
                    forwardStream(std::move(runtime), *stop, id,
                                  std::move(stream), std::move(subscription), heartbeat);
                } catch (...) {
                    // No exception may escape an SSE forwarding thread.
                }
                done->store(true);
            });
        workers_.push_back(std::move(worker));
    }

private:
    struct Worker {
        std::thread thread;
        std::shared_ptr<std::atomic_bool> stop;
        std::shared_ptr<std::atomic_bool> done;
    };
    std::mutex mutex_;
    std::vector<Worker> workers_;
};

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
    auto workers = std::make_shared<StreamWorkers>(options.maxConnections);
    connections->limit = options.maxConnections;
    app.registerHandler(std::move(path),
        [runtime = std::move(runtime), authorize = std::move(authorize),
         connections, workers, heartbeat = options.heartbeat](
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
            if (runtime->isStopping()) {
                reply(errorResponse(drogon::k503ServiceUnavailable, "Job Runtime is stopping"), callback);
                return;
            }
            const auto snapshot = runtime->find(*id);
            if (!snapshot) {
                reply(errorResponse(drogon::k404NotFound, "Unknown job"), callback);
                return;
            }
            // EventSource will not reconnect after HTTP 204. Completed Jobs
            // have a stable snapshot available via the ordinary JSON API.
            if (isTerminalJobStatus(snapshot->status)) {
                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k204NoContent);
                reply(std::move(response), callback);
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
                if (auto* diagnostics = runtime->runtimeDiagnostics()) {
                    diagnostics->recordJobSseRejected();
                }
                reply(errorResponse(drogon::k503ServiceUnavailable, "Too many SSE connections"), callback);
                return;
            }
            // Subscribe before handing over the HTTP response, so events between
            // validation and the asynchronous callback are not lost.
            auto subscription = runtime->events().subscribe(*id, cursor);
            auto lease = std::make_shared<ConnectionLease>();
            lease->count = connections;
            lease->diagnostics = runtime->runtimeDiagnostics();
            if (lease->diagnostics) {
                lease->diagnostics->recordJobSseOpened();
            }
            auto response = drogon::HttpResponse::newAsyncStreamResponse(
                [runtime, workers, jobId = *id, heartbeat,
                 lease = std::move(lease),
                 subscription = std::make_shared<JobEventStream::Subscription>(
                     std::move(subscription))](drogon::ResponseStreamPtr stream) mutable
                {
                    // Transfer the lease out of the response's callback.
                    // Drogon may retain this callback for the response lifetime,
                    // which must NOT keep the active connection gauge elevated.
                    auto workerLease = std::move(lease);
                    if (!stream) {
                        return;
                    }
                    try {
                        workers->start(runtime, jobId, std::move(stream),
                                       std::move(*subscription), heartbeat,
                                       std::move(workerLease));
                    } catch (...) {
                        // Thread construction failure releases the lease via RAII.
                    }
                }, true);
            response->setContentTypeString("text/event-stream; charset=utf-8");
            response->addHeader("Cache-Control", "no-cache, no-transform");
            response->addHeader("X-Accel-Buffering", "no");
            response->setExpiredTime(-1);
            callback(response);
        }, {drogon::Get});
}

} // namespace drogular