#pragma once

#include <any>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace drogular {

class RuntimeDiagnostics;
class JobEventStream;

using JobId = std::uint64_t;

enum class JobStatus {
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled
};

struct JobError {
    std::string code;
    std::string message;
};

struct JobSnapshot {
    JobId id = 0;
    std::string name;
    JobStatus status = JobStatus::Queued;
    unsigned progress = 0;
    std::string message;
    // Legacy message retained for existing consumers.
    std::string error;
    std::optional<JobError> failure;
    // Value is present only on successful completion. Copyable C++ values only.
    std::any result;
    template <typename T>
    const T* resultAs() const noexcept {
        return std::any_cast<T>(&result);
    }
    bool cancellationRequested = false;
    std::chrono::system_clock::time_point createdAt{};
    std::optional<std::chrono::system_clock::time_point> startedAt;
    std::optional<std::chrono::system_clock::time_point> finishedAt;
    std::optional<std::chrono::milliseconds> queueDuration;
    std::optional<std::chrono::milliseconds> executionDuration;
};

class JobCancelled final : public std::runtime_error {
public:
    JobCancelled() : std::runtime_error("Job cancelled") {}
};

/** Throw to fail a Job with a stable machine-readable error code. */
class JobFailure final : public std::runtime_error {
public:
    JobFailure(std::string code, std::string message)
        : std::runtime_error(message), code_(std::move(code)) {
        if (code_.empty()) throw std::invalid_argument("Job failure code cannot be empty");
    }
    const std::string& code() const noexcept { return code_; }
private:
    std::string code_;
};

class JobContext {
public:
    bool cancellationRequested() const noexcept;
    void throwIfCancellationRequested() const;
    void reportProgress(unsigned percent, std::string message = {});
    // Store a copyable C++ result. The value is published only if the task
    // finishes successfully; cancellation/failure clears it.
    void setResult(std::any value);
    template <typename T>
    void setResult(T value) { setResult(std::any(std::move(value))); }

private:
    friend class JobRuntime;
    struct Shared;
    explicit JobContext(std::shared_ptr<Shared> shared);
    std::shared_ptr<Shared> shared_;
};

/** A process-local, bounded queue with cooperative cancellation.
 * No Drogon, HTTP, PWA, or application ownership is required.
 * All snapshots are copies and safe to consume after a task completes.
 */
class JobRuntime {
public:
    using Task = std::function<void(JobContext&)>;

    explicit JobRuntime(std::size_t workers = 2,
                        std::size_t maxQueued = 1024,
                        RuntimeDiagnostics* diagnostics = nullptr);
    ~JobRuntime();
    JobRuntime(const JobRuntime&) = delete;
    JobRuntime& operator=(const JobRuntime&) = delete;

    // Throws std::invalid_argument for an empty task, std::runtime_error if
    // stopped or full. Rejections never create an inspectable Job.
    JobId submit(std::string name, Task task);
    std::optional<JobSnapshot> find(JobId id) const;
    std::vector<JobSnapshot> list() const;
    // Returns false only for an unknown Job. Terminal Jobs are unchanged.
    bool requestCancel(JobId id);
    // Reject new jobs, cancel queued jobs, request cancellation of running
    // jobs and join all workers. Running tasks must cooperate to stop promptly.
    void shutdown() noexcept;
    std::size_t workerCount() const noexcept;
    RuntimeDiagnostics* runtimeDiagnostics() const noexcept;
    // Transport-neutral observable event stream. Outlives individual subscriptions.
    JobEventStream& events() noexcept;

private:
    struct Entry;
    void workerLoop();
    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::map<JobId, std::shared_ptr<Entry>> entries_;
    std::deque<std::shared_ptr<Entry>> queue_;
    std::vector<std::thread> workers_;
    std::size_t maxQueued_;
    RuntimeDiagnostics* diagnostics_;
    std::unique_ptr<JobEventStream> events_;
    JobId nextId_ = 1;
    bool stopping_ = false;
};

} // namespace drogular