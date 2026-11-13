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
    std::condition_variable pauseChanged;
    std::optional<std::chrono::steady_clock::time_point> pausedSince;
    RuntimeDiagnostics* diagnostics = nullptr;
    JobEventStream* events = nullptr;
    bool resumable = false;
};

namespace {

struct JobYield final {};

}

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

void JobContext::waitIfPaused() const {
    throwIfCancellationRequested();
    std::unique_lock lock(shared_->mutex);
    if (!shared_->snapshot.pauseRequested || shared_->snapshot.status != JobStatus::Running) {
        return;
    }
    if (shared_->resumable) {
        lock.unlock();
        throw JobYield{}; // Unwind stack and release worker; caller preserves checkpoint.
    }
    shared_->snapshot.status = JobStatus::Paused;
    shared_->pausedSince = std::chrono::steady_clock::now();
    if (shared_->diagnostics) {
        shared_->diagnostics->recordJobPaused();
    }
    if (shared_->events) {
        shared_->events->publish(JobEventKind::Paused, shared_->snapshot);
    }
    shared_->pauseChanged.wait(lock, [this] {
        return !shared_->snapshot.pauseRequested || shared_->cancel.load(std::memory_order_acquire);
    });
    if (shared_->pausedSince) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - *shared_->pausedSince);
        shared_->snapshot.pausedDuration += elapsed;
        shared_->pausedSince.reset();
        if (shared_->diagnostics) {
            shared_->diagnostics->recordJobResumed(elapsed.count());
        }
    }
    shared_->snapshot.status = JobStatus::Running;
    if (shared_->events && !shared_->cancel.load(std::memory_order_acquire)) {
        shared_->events->publish(JobEventKind::Resumed, shared_->snapshot);
    }
    lock.unlock();
    throwIfCancellationRequested();
}

void JobContext::reportProgress(unsigned percent, std::string message) {
    waitIfPaused();
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
    waitIfPaused();
    std::lock_guard lock(shared_->mutex);
    if (shared_->snapshot.status != JobStatus::Running) {
        return;
    }
    shared_->snapshot.result = std::move(value);
}

JobRuntime::JobRuntime(
    std::size_t workers,
    std::size_t maxQueued,
    RuntimeDiagnostics* diagnostics,
    std::size_t maxRetainedTerminal
    )
    : maxQueued_(maxQueued), maxRetainedTerminal_(maxRetainedTerminal),
      diagnostics_(diagnostics),
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
    return submitImpl(std::move(name), std::move(task), false);
}

JobId JobRuntime::submitResumable(std::string name, Task task) {
    return submitImpl(std::move(name), std::move(task), true);
}

JobId JobRuntime::submitImpl(std::string name, Task task, bool resumable) {
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
    shared->resumable = resumable;
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
    pruneTerminalJobsLocked(maxRetainedTerminal_);

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
    bool cancelledImmediately = false;
    {
        std::lock_guard jobLock(shared.mutex);
        if (shared.snapshot.status == JobStatus::Queued) {
            shared.snapshot.pauseRequested = false;
            shared.snapshot.cancellationRequested = true;
            shared.cancel.store(true, std::memory_order_release);
            shared.snapshot.status = JobStatus::Cancelled;
            shared.snapshot.finishedAt = std::chrono::system_clock::now();
            // Remove cancelled entries immediately: their queue capacity should
            // be available to new jobs even while every worker is occupied.
            queue_.erase(std::remove(queue_.begin(), queue_.end(), it->second), queue_.end());
            if (diagnostics_) {
                diagnostics_->recordJobCancelled(false);
            }
            events_->publish(JobEventKind::Cancelled, shared.snapshot);
            cancelledImmediately = true;
        } else if (shared.snapshot.status == JobStatus::Paused && shared.resumable) {
            shared.snapshot.pauseRequested = false;
            shared.snapshot.cancellationRequested = true;
            shared.cancel.store(true, std::memory_order_release);
            if (shared.pausedSince) {
                shared.snapshot.pausedDuration += std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - *shared.pausedSince);
                shared.pausedSince.reset();
            }
            shared.snapshot.status = JobStatus::Cancelled;
            shared.snapshot.finishedAt = std::chrono::system_clock::now();
            if (diagnostics_) {
                diagnostics_->recordJobCancelledPaused();
            }
            events_->publish(JobEventKind::Cancelled, shared.snapshot);
            cancelledImmediately = true;
        } else if (shared.snapshot.status == JobStatus::Running || shared.snapshot.status == JobStatus::Paused) {
            if (!shared.cancel.exchange(true, std::memory_order_acq_rel)) {
                shared.snapshot.cancellationRequested = true;
                if (diagnostics_) {
                    diagnostics_->recordJobCancellationRequested();
                }
                events_->publish(JobEventKind::CancellationRequested, shared.snapshot);
            }
            shared.pauseChanged.notify_all();
        }
    }
    if (cancelledImmediately) {
        pruneTerminalJobsLocked(maxRetainedTerminal_);
    }

    return true;
}

bool JobRuntime::requestPause(JobId id) {
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(id);
    if (it == entries_.end()) {
        return false;
    }

    auto& shared = *it->second->shared;
    std::lock_guard jobLock(shared.mutex);
    if (shared.snapshot.status == JobStatus::Queued ||
        shared.snapshot.status == JobStatus::Running ||
        shared.snapshot.status == JobStatus::Paused
    ) {
        if (!shared.snapshot.cancellationRequested && !shared.snapshot.pauseRequested) {
            shared.snapshot.pauseRequested = true;
            if (diagnostics_) {
                diagnostics_->recordJobPauseRequested();
            }
            events_->publish(JobEventKind::PauseRequested, shared.snapshot);
        }
    }

    return true;
}

bool JobRuntime::requestResume(JobId id) {
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(id);
    if (it == entries_.end()) {
        return false;
    }

    auto& shared = *it->second->shared;
    std::lock_guard jobLock(shared.mutex);
    if (shared.snapshot.pauseRequested && !shared.snapshot.cancellationRequested && !stopping_) {
        if (shared.resumable && shared.snapshot.status == JobStatus::Paused && queue_.size() >= maxQueued_) {
            return false; // Keep paused until capacity becomes available.
        }
        shared.snapshot.pauseRequested = false;
        if (diagnostics_) {
            diagnostics_->recordJobResumeRequested();
        }
        if (shared.resumable && shared.snapshot.status == JobStatus::Paused) {
            auto elapsed = std::chrono::milliseconds{0};
            if (shared.pausedSince) {
                elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - *shared.pausedSince);
                shared.snapshot.pausedDuration += elapsed;
                shared.pausedSince.reset();
            }
            shared.snapshot.status = JobStatus::Queued;
            queue_.push_back(it->second); // Back of FIFO; do not jump ahead of waiting jobs.
            if (diagnostics_) {
                diagnostics_->recordJobRequeued(elapsed.count());
            }
            events_->publish(JobEventKind::Resumed, shared.snapshot);
        }
        // Legacy queued pauses did not leave the queue; publish a single
        // resume event, not a duplicate for resumable requeue transitions.
        else if (shared.snapshot.status == JobStatus::Queued) {
            events_->publish(JobEventKind::Resumed, shared.snapshot);
        }
        shared.pauseChanged.notify_all();
        ready_.notify_all();
    }

    return true;
}

void JobRuntime::workerLoop() {
    while (true) {
        std::shared_ptr<Entry> entry;
        {
            std::unique_lock lock(mutex_);
            ready_.wait(lock, [this] {
                return stopping_ || std::any_of(queue_.begin(), queue_.end(), [](const auto& entry) {
                    std::lock_guard lock(entry->shared->mutex);
                    return !entry->shared->snapshot.pauseRequested;
                });
            });
            if (queue_.empty()) {
                if (stopping_) {
                    return;
                }
                continue;
            }
            const auto ready = std::find_if(queue_.begin(), queue_.end(), [](const auto& candidate) {
                std::lock_guard lock(candidate->shared->mutex);
                return !candidate->shared->snapshot.pauseRequested;
            });
            if (ready == queue_.end()) {
                continue;
            }
            entry = *ready;
            queue_.erase(ready);
        }

        auto& shared = *entry->shared;
        {
            std::lock_guard lock(shared.mutex);
            if (shared.snapshot.status == JobStatus::Cancelled) {
                continue;
            }

            shared.snapshot.status = JobStatus::Running;
            const auto now = std::chrono::system_clock::now();
            const bool firstStart = !shared.snapshot.startedAt;
            if (firstStart) shared.snapshot.startedAt = now;
            if (firstStart) {
                shared.snapshot.queueDuration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - shared.snapshot.createdAt);
            }

            if (diagnostics_) {
                if (firstStart) {
                    diagnostics_->recordJobQueueWait(
                        static_cast<std::uint64_t>(shared.snapshot.queueDuration->count())
                    );
                }
                diagnostics_->recordJobStarted();
            }
            events_->publish(JobEventKind::Started, shared.snapshot);
        }

        bool yielded = false;
        JobStatus outcome = JobStatus::Completed;
        std::string error;
        std::string errorCode;
        try {
            JobContext ctx(entry->shared);
            ctx.waitIfPaused();
            entry->task(ctx);
            ctx.waitIfPaused();
        } catch (const JobYield&) {
            yielded = true;
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

        if (yielded) {
            // Runtime lock precedes job lock everywhere, including Resume/Cancel.
            std::lock_guard runtimeLock(mutex_);
            std::lock_guard jobLock(shared.mutex);
            if (shared.cancel.load(std::memory_order_acquire) || stopping_) {
                shared.snapshot.status = JobStatus::Cancelled;
                shared.snapshot.finishedAt = std::chrono::system_clock::now();
                shared.snapshot.result.reset();
                if (diagnostics_) {
                    diagnostics_->recordJobCancelled(true);
                }
                events_->publish(JobEventKind::Cancelled, shared.snapshot);
            } else if (shared.snapshot.pauseRequested) {
                shared.snapshot.status = JobStatus::Paused;
                shared.pausedSince = std::chrono::steady_clock::now();
                if (diagnostics_) {
                    diagnostics_->recordJobYielded();
                }
                events_->publish(JobEventKind::Paused, shared.snapshot);
            } else {
                shared.snapshot.status = JobStatus::Queued;
                queue_.push_back(entry);
                if (diagnostics_) {
                    diagnostics_->recordJobRunningRequeued();
                }
                events_->publish(JobEventKind::Queued, shared.snapshot);
                ready_.notify_one();
            }
            continue;
        }
        {
            std::lock_guard lock(shared.mutex);
            shared.snapshot.status = outcome;
            shared.snapshot.pauseRequested = false;
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
        {
            // Always take runtime mutex before a job mutex, matching Cancel/Resume.
            std::lock_guard lock(mutex_);
            pruneTerminalJobsLocked(maxRetainedTerminal_);
        }
    }
}

std::size_t JobRuntime::pruneTerminalJobsLocked(std::size_t keepLatest) {
    // IDs are monotonically increasing; maps iterate from oldest to newest.
    // Only terminal entries are considered. Never remove a dispatched job.
    std::size_t terminals = 0;
    for (const auto& [id, entry] : entries_) {
        std::lock_guard lock(entry->shared->mutex);
        const auto state = entry->shared->snapshot.status;
        if (state == JobStatus::Completed || state == JobStatus::Failed || state == JobStatus::Cancelled) {
            ++terminals;
        }
    }
    std::size_t removed = 0;
    for (auto it = entries_.begin(); it != entries_.end() && terminals > keepLatest;) {
        std::lock_guard lock(it->second->shared->mutex);
        const auto state = it->second->shared->snapshot.status;
        if (state == JobStatus::Completed || state == JobStatus::Failed || state == JobStatus::Cancelled) {
            // Queued cancellation removes the entry from queue_ immediately.
            // Other terminal jobs are no longer present in the queue.
            it = entries_.erase(it);
            --terminals;
            ++removed;
        } else {
            ++it;
        }
    }
    return removed;
}

std::size_t JobRuntime::pruneTerminalJobs(std::size_t keepLatest) {
    std::lock_guard lock(mutex_);
    return pruneTerminalJobsLocked(keepLatest);
}

bool JobRuntime::isStopping() const noexcept {
    std::lock_guard lock(mutex_);
    return stopping_;
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
                    shared.snapshot.pauseRequested = false;
                    shared.cancel.store(true, std::memory_order_release);
                    shared.snapshot.cancellationRequested = true;
                    shared.snapshot.status = JobStatus::Cancelled;
                    shared.snapshot.finishedAt = std::chrono::system_clock::now();
                    if (diagnostics_) {
                        diagnostics_->recordJobCancelled(false);
                    }
                    events_->publish(JobEventKind::Cancelled, shared.snapshot);
                } else if (shared.snapshot.status == JobStatus::Paused && shared.resumable) {
                    shared.cancel.store(true, std::memory_order_release);
                    shared.snapshot.cancellationRequested = true;
                    shared.snapshot.pauseRequested = false;
                    shared.snapshot.status = JobStatus::Cancelled;
                    shared.snapshot.finishedAt = std::chrono::system_clock::now();
                    if (shared.pausedSince) {
                        shared.snapshot.pausedDuration += std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - *shared.pausedSince);
                        shared.pausedSince.reset();
                    }
                    if (diagnostics_) {
                        diagnostics_->recordJobCancelledPaused();
                    }
                    events_->publish(JobEventKind::Cancelled, shared.snapshot);
                } else if (shared.snapshot.status == JobStatus::Running ||
                           shared.snapshot.status == JobStatus::Paused
                ) {
                    shared.cancel.store(true, std::memory_order_release);
                    shared.snapshot.cancellationRequested = true;
                    shared.pauseChanged.notify_all();
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