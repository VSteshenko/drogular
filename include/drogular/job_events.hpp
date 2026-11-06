#pragma once

#include <drogular/job_runtime.hpp>

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace drogular {

enum class JobEventKind {
    Queued,
    Started,
    Progress,
    CancellationRequested,
    Completed,
    Failed,
    Cancelled
};

struct JobEvent {
    std::uint64_t sequence = 0;
    JobEventKind kind = JobEventKind::Queued;
    JobSnapshot job;
};

struct JobEventBatch {
    std::vector<JobEvent> events;
    // Indicates a cursor is too old or a slow subscriber overflowed.
    // Consumers must resynchronize using JobRuntime::find/list.
    bool gap = false;
};

/**
 * Thread-safe, bounded process-local event stream, independent of HTTP.
 * Each subscription buffers events without blocking worker threads.
 */
class JobEventStream {
    struct Subscriber;
    struct State;

public:
    class Subscription {
    public:
        Subscription() = default;
        Subscription(Subscription&&) noexcept = default;
        Subscription& operator=(Subscription&&) noexcept;
        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;
        ~Subscription();

        JobEventBatch drain();
        // Wait until events arrive or timeout; returns a possibly empty batch.
        JobEventBatch waitFor(std::chrono::milliseconds timeout);
        explicit operator bool() const noexcept {
            return static_cast<bool>(subscriber_);
        }

    private:
        friend class JobEventStream;

        Subscription(std::shared_ptr<State>, std::shared_ptr<Subscriber>);
        std::shared_ptr<State> state_;
        std::shared_ptr<Subscriber> subscriber_;
    };

    explicit JobEventStream(std::size_t replayCapacity = 256,
                            std::size_t subscriberCapacity = 256,
                            RuntimeDiagnostics* diagnostics = nullptr);
    JobEventStream(const JobEventStream&) = delete;
    JobEventStream& operator=(const JobEventStream&) = delete;

    // Replay events with sequence > afterSequence; nullopt starts with live events.
    // A job filter is applied to both replay and live delivery.
    Subscription subscribe(std::optional<JobId> job = std::nullopt,
                           std::optional<std::uint64_t> afterSequence = std::nullopt);
    std::uint64_t latestSequence() const;
    void publish(JobEventKind kind, JobSnapshot snapshot);

private:
    std::shared_ptr<State> state_;
};

} // namespace drogular