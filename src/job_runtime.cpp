#include <drogular/job_runtime.hpp>
#include <drogular/job_events.hpp>
#include <drogular/runtime_diagnostics.hpp>

#include <algorithm>
#include <exception>
#include <utility>

namespace drogular {

struct JobContext::Shared {
    mutable std::mutex mutex;
    JobSnapshot snapshot;
    std::atomic<bool> cancel{false};
    RuntimeDiagnostics* diagnostics = nullptr;
    JobEventStream* events = nullptr;
};

struct JobRuntime::Entry {
    std::shared_ptr<JobContext::Shared> shared;
    Task task;
};

JobContext::JobContext(std::shared_ptr<Shared> shared)
    : shared_(std::move(shared))
{
}

bool JobContext::cancellationRequested() const noexcept {
    return shared_->cancel.load(std::memory_order_acquire);
}

void JobContext::throwIfCancellationRequested() const {
    if (cancellationRequested()) {
        throw JobCancelled();
    }
}

void JobContext::reportProgress(unsigned percent, std::string message) {
    if (percent > 100) {
        throw std::out_of_range("Job progress must be 0..100");
    }

    throwIfCancellationRequested();

    {
        std::lock_guard lock(shared_->mutex);
        if (shared_->snapshot.status != JobStatus::Running) return;
        shared_->snapshot.progress = percent;
        shared_->snapshot.message = std::move(message);
    }

    if (shared_->diagnostics) {
        shared_->diagnostics->recordJobProgress();
    }
    if (shared_->events) {
        std::lock_guard lock(shared_->mutex);
        shared_->events->publish(JobEventKind::Progress, shared_->snapshot);
    }
}

void JobContext::setResult(std::any value) {
    throwIfCancellationRequested();
    std::lock_guard lock(shared_->mutex);
    if (shared_->snapshot.status != JobStatus::Running) {
        return;
    }
    shared_->snapshot.result = std::move(value);
}

JobRuntime::JobRuntime(
    std::size_t workers,
    std::size_t maxQueued,
    RuntimeDiagnostics* diagnostics
    )
    : maxQueued_(maxQueued), diagnostics_(diagnostics),
      events_(std::make_unique<JobEventStream>(256, 256, diagnostics))
{
    if (workers == 0 || maxQueued == 0) {
        throw std::invalid_argument("JobRuntime requires workers and queue capacity");
    }
    workers_.reserve(workers);
    if (diagnostics_) {
        diagnostics_->registerJobQueueCapacity(maxQueued_);
    }
    try {
        for (std::size_t i = 0; i < workers; ++i) {
            workers_.emplace_back([this] { workerLoop(); });
            if (diagnostics_) {
                diagnostics_->registerJobWorkers(1);
            }
        }
    } catch (...) {
        shutdown();
        throw;
    }
}

JobRuntime::~JobRuntime() {
    shutdown();
}

JobId JobRuntime::submit(std::string name, Task task) {
    if (!task) {
        throw std::invalid_argument("Job task cannot be empty");
    }

    std::lock_guard lock(mutex_);

    if (stopping_) {
        throw std::runtime_error("JobRuntime is stopped");
    }
    if (queue_.size() >= maxQueued_) {
        if (diagnostics_) {
            diagnostics_->recordJobRejected();
        }
        throw std::runtime_error("Job queue is full");
    }

    auto shared = std::make_shared<JobContext::Shared>();
    shared->diagnostics = diagnostics_;
    shared->events = events_.get();
    shared->snapshot.id = nextId_++;
    shared->snapshot.name = std::move(name);
    shared->snapshot.createdAt = std::chrono::system_clock::now();
    auto entry = std::make_shared<Entry>();
    entry->shared = std::move(shared);
    entry->task = std::move(task);
    const auto id = entry->shared->snapshot.id;
    entries_.emplace(id, entry);
    queue_.push_back(entry);

    if (diagnostics_) {
        diagnostics_->recordJobSubmitted();
    }

    events_->publish(JobEventKind::Queued, entry->shared->snapshot);
    ready_.notify_one();
    return id;
}

std::optional<JobSnapshot> JobRuntime::find(JobId id) const {
    std::shared_ptr<Entry> entry;
    {
        std::lock_guard lock(mutex_);
        const auto it = entries_.find(id);
        if (it == entries_.end()) {
            return std::nullopt;
        }
        entry = it->second;
    }
    std::lock_guard lock(entry->shared->mutex);
    return entry->shared->snapshot;
}

std::vector<JobSnapshot> JobRuntime::list() const {
    std::vector<std::shared_ptr<Entry>> entries;
    {
        std::lock_guard lock(mutex_);
        for (const auto& [id, entry] : entries_) {
            entries.push_back(entry);
        }
    }
    std::vector<JobSnapshot> result;
    result.reserve(entries.size());
    for (const auto& entry : entries) {
        std::lock_guard lock(entry->shared->mutex);
        result.push_back(entry->shared->snapshot);
    }

    return result;
}

bool JobRuntime::requestCancel(JobId id) {
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(id);
    if (it == entries_.end()) {
        return false;
    }
    auto& shared = *it->second->shared;
    std::lock_guard jobLock(shared.mutex);
    if (shared.snapshot.status == JobStatus::Queued) {
        shared.snapshot.cancellationRequested = true;
        shared.cancel.store(true, std::memory_order_release);
        shared.snapshot.status = JobStatus::Cancelled;
        shared.snapshot.finishedAt = std::chrono::system_clock::now();
        if (diagnostics_) {
            diagnostics_->recordJobCancelled(false);
        }
        events_->publish(JobEventKind::Cancelled, shared.snapshot);
    } else if (shared.snapshot.status == JobStatus::Running) {
        if (!shared.cancel.exchange(true, std::memory_order_acq_rel)) {
            shared.snapshot.cancellationRequested = true;
            if (diagnostics_) {
                diagnostics_->recordJobCancellationRequested();
            }
            events_->publish(JobEventKind::CancellationRequested, shared.snapshot);
        }
    }

    return true;
}

void JobRuntime::workerLoop() {
    while (true) {
        std::shared_ptr<Entry> entry;
        {
            std::unique_lock lock(mutex_);
            ready_.wait(lock, [this] {
                return stopping_ || !queue_.empty();
            });
            if (queue_.empty()) {
                if (stopping_) {
                    return;
                }
                continue;
            }
            entry = queue_.front();
            queue_.pop_front();
        }

        auto& shared = *entry->shared;
        {
            std::lock_guard lock(shared.mutex);
            if (shared.snapshot.status == JobStatus::Cancelled) {
                continue;
            }

            shared.snapshot.status = JobStatus::Running;
            shared.snapshot.startedAt = std::chrono::system_clock::now();
            shared.snapshot.queueDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                *shared.snapshot.startedAt - shared.snapshot.createdAt);

            if (diagnostics_) {
                diagnostics_->recordJobQueueWait(
                    static_cast<std::uint64_t>(shared.snapshot.queueDuration->count()));
                diagnostics_->recordJobStarted();
            }
            events_->publish(JobEventKind::Started, shared.snapshot);
        }

        JobStatus outcome = JobStatus::Completed;
        std::string error;
        std::string errorCode;
        try {
            JobContext ctx(entry->shared);
            ctx.throwIfCancellationRequested();
            entry->task(ctx);
            ctx.throwIfCancellationRequested();
        } catch (const JobCancelled&) {
            outcome = JobStatus::Cancelled;
        } catch (const JobFailure& e) {
            outcome = JobStatus::Failed;
            errorCode = e.code();
            error = e.what();
        } catch (const std::exception& e) {
            outcome = JobStatus::Failed;
            errorCode = "exception";
            error = e.what();
        } catch (...) {
            outcome = JobStatus::Failed;
            errorCode = "unknown_exception";
            error = "Unknown Job exception";
        }

        {
            std::lock_guard lock(shared.mutex);
            shared.snapshot.status = outcome;
            shared.snapshot.error = std::move(error);
            if (outcome == JobStatus::Failed) {
                shared.snapshot.failure = JobError {
                    std::move(errorCode),
                    shared.snapshot.error
                };
                shared.snapshot.result.reset();
            } else if (outcome == JobStatus::Cancelled) {
                shared.snapshot.result.reset();
            }

            shared.snapshot.finishedAt = std::chrono::system_clock::now();
            shared.snapshot.executionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                *shared.snapshot.finishedAt - *shared.snapshot.startedAt);

            if (outcome == JobStatus::Completed) {
                shared.snapshot.progress = 100;
            }
            events_->publish(
                outcome == JobStatus::Completed ? JobEventKind::Completed :
                outcome == JobStatus::Failed ? JobEventKind::Failed : JobEventKind::Cancelled,
                shared.snapshot);
            if (diagnostics_) {
                diagnostics_->recordJobExecution(
                    static_cast<std::uint64_t>(shared.snapshot.executionDuration->count()));
                switch (outcome) {
                    case JobStatus::Completed:
                        diagnostics_->recordJobCompleted();
                        break;
                    case JobStatus::Failed:
                        diagnostics_->recordJobFailed();
                        break;
                    case JobStatus::Cancelled:
                        diagnostics_->recordJobCancelled(true);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void JobRuntime::shutdown() noexcept {
    bool firstShutdown = false;
    {
        std::lock_guard lock(mutex_);
        if (!stopping_) {
            firstShutdown = true;
            stopping_ = true;
            for (const auto& [id, entry] : entries_) {
                auto& shared = *entry->shared;
                std::lock_guard jobLock(shared.mutex);
                if (shared.snapshot.status == JobStatus::Queued) {
                    shared.cancel.store(true, std::memory_order_release);
                    shared.snapshot.cancellationRequested = true;
                    shared.snapshot.status = JobStatus::Cancelled;
                    shared.snapshot.finishedAt = std::chrono::system_clock::now();
                    if (diagnostics_) {
                        diagnostics_->recordJobCancelled(false);
                    }
                    events_->publish(JobEventKind::Cancelled, shared.snapshot);
                } else if (shared.snapshot.status == JobStatus::Running) {
                    shared.cancel.store(true, std::memory_order_release);
                    shared.snapshot.cancellationRequested = true;
                }
            }
            queue_.clear();
        }
    }
    ready_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    if (firstShutdown && diagnostics_) {
        diagnostics_->unregisterJobWorkers(workers_.size());
        diagnostics_->unregisterJobQueueCapacity(maxQueued_);
    }
}

JobEventStream& JobRuntime::events() noexcept {
    return *events_;
}

RuntimeDiagnostics* JobRuntime::runtimeDiagnostics() const noexcept {
    return diagnostics_;
}

std::size_t JobRuntime::workerCount() const noexcept {
    return workers_.size();
}

} // namespace drogular