#pragma once

#include <cstddef>
#include <cstdint>

namespace drogular {

struct RequestRuntimeDiagnostics {
    std::uint64_t requests = 0;
    std::uint64_t actions = 0;
    std::uint64_t renderedActions = 0;
    std::uint64_t redirects = 0;
};

struct InteractionRuntimeDiagnostics {
    std::uint64_t requests = 0;
    std::uint64_t getRequests = 0;
    std::uint64_t postRequests = 0;
    std::uint64_t successfulResponses = 0;
    std::uint64_t clientErrorResponses = 0;
    std::uint64_t redirects = 0;
};

struct RenderingRuntimeDiagnostics {
    std::uint64_t componentsRendered = 0;
    std::uint64_t renderFailures = 0;
    std::size_t maximumDepth = 0;
};

struct ServiceScopeRuntimeDiagnostics {
    std::uint64_t resolutions = 0;
    std::uint64_t cacheHits = 0;
    std::uint64_t servicesCreated = 0;
};

struct JobRuntimeDiagnostics {
    std::uint64_t submitted = 0;
    std::uint64_t started = 0;
    std::uint64_t completed = 0;
    std::uint64_t failed = 0;
    std::uint64_t cancelled = 0;
    std::uint64_t cancellationRequests = 0;
    std::uint64_t paused = 0;
    std::uint64_t pauseRequests = 0;
    std::uint64_t resumeRequests = 0;
    std::uint64_t pauseTotalMs = 0;
    std::uint64_t pauseSamples = 0;
    std::uint64_t pauseAverageMs = 0;
    std::uint64_t progressUpdates = 0;
    std::uint64_t rejected = 0;
    std::uint64_t queued = 0;
    std::uint64_t running = 0;
    std::uint64_t workerCapacity = 0;
    std::uint64_t queueCapacity = 0;
    std::uint64_t peakQueued = 0;
    std::uint64_t peakRunning = 0;
    std::uint64_t queueWaitMaxMs = 0;
    std::uint64_t executionMaxMs = 0;
    std::uint64_t queueWaitSamples = 0;
    std::uint64_t executionSamples = 0;
    std::uint64_t queueWaitAverageMs = 0;
    std::uint64_t executionAverageMs = 0;
    // These ratios are percentages in basis points (10000 = 100%).
    std::uint64_t workerUtilizationBps = 0;
    std::uint64_t queueSaturationBps = 0;

    std::uint64_t queueWaitTotalMs = 0;
    std::uint64_t executionTotalMs = 0;
    std::uint64_t eventsPublished = 0;
    std::uint64_t eventGaps = 0;
    std::uint64_t activeSubscriptions = 0;
    std::uint64_t activeSseConnections = 0;
    std::uint64_t sseConnectionsOpened = 0;
    std::uint64_t sseConnectionsRejected = 0;
    std::uint64_t sseConnectionsClosed = 0;
};

struct RuntimeDiagnosticsSnapshot {
    JobRuntimeDiagnostics jobs;
    RequestRuntimeDiagnostics requests;
    InteractionRuntimeDiagnostics interactions;
    RenderingRuntimeDiagnostics rendering;
    ServiceScopeRuntimeDiagnostics serviceScopes;
};

/**
 * Thread-safe, runtime-neutral aggregate diagnostics.
 *
 * RuntimeDiagnostics deliberately has no dependency on Developer Tools,
 * ApplicationInspection, JSON, or HTTP request objects. Runtime subsystems
 * record inexpensive aggregate events; inspection and observability layers
 * consume immutable snapshots.
 */
class RuntimeDiagnostics {
public:
    RuntimeDiagnostics();
    ~RuntimeDiagnostics();

    RuntimeDiagnostics(const RuntimeDiagnostics&) = delete;
    RuntimeDiagnostics& operator=(const RuntimeDiagnostics&) = delete;
    RuntimeDiagnostics(RuntimeDiagnostics&&) = delete;
    RuntimeDiagnostics& operator=(RuntimeDiagnostics&&) = delete;

    void recordRequest() noexcept;
    void recordAction() noexcept;
    void recordRenderedAction() noexcept;
    void recordRedirect() noexcept;

    void recordInteractionRequest() noexcept;
    void recordInteractionGetRequest() noexcept;
    void recordInteractionPostRequest() noexcept;
    void recordInteractionSuccessfulResponse() noexcept;
    void recordInteractionClientErrorResponse() noexcept;
    void recordInteractionRedirect() noexcept;

    void recordComponentRendered(std::size_t depth = 0) noexcept;
    void recordRenderFailure() noexcept;

    void recordServiceScopeResolution() noexcept;
    void recordServiceScopeCacheHit() noexcept;
    void recordServiceScopeServiceCreated() noexcept;

    void recordJobSubmitted() noexcept;
    void recordJobStarted() noexcept;
    void recordJobCompleted() noexcept;
    void recordJobFailed() noexcept;
    void recordJobCancelled(bool wasRunning) noexcept;
    void recordJobCancellationRequested() noexcept;
    void recordJobPauseRequested() noexcept;
    void recordJobResumeRequested() noexcept;
    void recordJobPaused() noexcept;
    void recordJobYielded() noexcept;
    void recordJobRequeued(std::uint64_t pausedMs) noexcept;
    void recordJobRunningRequeued() noexcept;
    void recordJobCancelledPaused() noexcept;
    void recordJobResumed(std::uint64_t milliseconds) noexcept;
    void recordJobProgress() noexcept;
    void recordJobRejected() noexcept;
    void registerJobWorkers(std::size_t count) noexcept;
    void registerJobQueueCapacity(std::size_t count) noexcept;
    void unregisterJobQueueCapacity(std::size_t count) noexcept;
    void unregisterJobWorkers(std::size_t count) noexcept;
    void recordJobQueueWait(std::uint64_t milliseconds) noexcept;
    void recordJobExecution(std::uint64_t milliseconds) noexcept;
    void recordJobEventPublished() noexcept;
    void recordJobEventGap() noexcept;
    void recordJobSubscriptionOpened() noexcept;
    void recordJobSubscriptionClosed() noexcept;
    void recordJobSseOpened() noexcept;
    void recordJobSseRejected() noexcept;
    void recordJobSseClosed() noexcept;

    RuntimeDiagnosticsSnapshot snapshot() const noexcept;

private:
    struct State;
    State* state_;
};

} // namespace drogular