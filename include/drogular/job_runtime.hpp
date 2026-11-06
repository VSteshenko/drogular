#pragma once

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

using JobId = std::uint64_t;

enum class JobStatus {
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled
};

struct JobSnapshot {
    JobId id = 0;
    std::string name;
    JobStatus status = JobStatus::Queued;
    unsigned progress = 0;
    std::string message;
    std::string error;
    bool cancellationRequested = false;
    std::chrono::system_clock::time_point createdAt{};
    std::optional<std::chrono::system_clock::time_point> startedAt;
    std::optional<std::chrono::system_clock::time_point> finishedAt;
};

class JobCancelled final : public std::runtime_error {
public:
    JobCancelled() : std::runtime_error("Job cancelled") {}
};

class JobContext {
public:
    bool cancellationRequested() const noexcept;
    void throwIfCancellationRequested() const;
    void reportProgress(unsigned percent, std::string message = {});

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
    JobId nextId_ = 1;
    bool stopping_ = false;
};

} // namespace drogular