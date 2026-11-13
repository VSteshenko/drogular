#include <drogular/runtime_diagnostics.hpp>

#include <atomic>

namespace drogular {

namespace {

template<typename T>
void updateMaximum(
    std::atomic<T>& maximum,
    T value
) noexcept {
    auto current = maximum.load(std::memory_order_relaxed);
    while (
        current < value &&
        !maximum.compare_exchange_weak(
            current,
            value,
            std::memory_order_relaxed,
            std::memory_order_relaxed
        )
    ) {
    }
}

} // namespace

struct RuntimeDiagnostics::State {
    std::atomic<std::uint64_t> jobSubmitted{0};
    std::atomic<std::uint64_t> jobStarted{0};
    std::atomic<std::uint64_t> jobCompleted{0};
    std::atomic<std::uint64_t> jobFailed{0};
    std::atomic<std::uint64_t> jobCancelled{0};
    std::atomic<std::uint64_t> jobCancellationRequests{0};
    std::atomic<std::uint64_t> jobPaused{0};
    std::atomic<std::uint64_t> jobPauseRequests{0};
    std::atomic<std::uint64_t> jobResumeRequests{0};
    std::atomic<std::uint64_t> jobPauseTotalMs{0};
    std::atomic<std::uint64_t> jobPauseSamples{0};
    std::atomic<std::uint64_t> jobProgress{0};
    std::atomic<std::uint64_t> jobRejected{0};
    std::atomic<std::uint64_t> jobQueued{0};
    std::atomic<std::uint64_t> jobRunning{0};
    std::atomic<std::uint64_t> jobWorkerCapacity{0};
    std::atomic<std::uint64_t> jobQueueCapacity{0};
    std::atomic<std::uint64_t> jobPeakQueued{0};
    std::atomic<std::uint64_t> jobPeakRunning{0};
    std::atomic<std::uint64_t> jobQueueWaitMaxMs{0};
    std::atomic<std::uint64_t> jobExecutionMaxMs{0};
    std::atomic<std::uint64_t> jobQueueWaitSamples{0};
    std::atomic<std::uint64_t> jobExecutionSamples{0};
    std::atomic<std::uint64_t> jobQueueWaitTotalMs{0};
    std::atomic<std::uint64_t> jobExecutionTotalMs{0};
    std::atomic<std::uint64_t> jobEventsPublished{0};
    std::atomic<std::uint64_t> jobEventGaps{0};
    std::atomic<std::uint64_t> jobSubscriptions{0};
    std::atomic<std::uint64_t> jobSseConnections{0};
    std::atomic<std::uint64_t> jobSseOpened{0};
    std::atomic<std::uint64_t> jobSseRejected{0};
    std::atomic<std::uint64_t> jobSseClosed{0};
    std::atomic<std::uint64_t> requests{0};
    std::atomic<std::uint64_t> actions{0};
    std::atomic<std::uint64_t> renderedActions{0};
    std::atomic<std::uint64_t> redirects{0};

    std::atomic<std::uint64_t> interactionRequests{0};
    std::atomic<std::uint64_t> interactionGetRequests{0};
    std::atomic<std::uint64_t> interactionPostRequests{0};
    std::atomic<std::uint64_t> interactionSuccessfulResponses{0};
    std::atomic<std::uint64_t> interactionClientErrorResponses{0};
    std::atomic<std::uint64_t> interactionRedirects{0};

    std::atomic<std::uint64_t> componentsRendered{0};
    std::atomic<std::uint64_t> renderFailures{0};
    std::atomic<std::size_t> maximumRenderDepth{0};

    std::atomic<std::uint64_t> serviceScopeResolutions{0};
    std::atomic<std::uint64_t> serviceScopeCacheHits{0};
    std::atomic<std::uint64_t> serviceScopeServicesCreated{0};
};

RuntimeDiagnostics::RuntimeDiagnostics()
    : state_(new State) {
}

RuntimeDiagnostics::~RuntimeDiagnostics() {
    delete state_;
}

void RuntimeDiagnostics::recordRequest() noexcept {
    state_->requests.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordAction() noexcept {
    state_->actions.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordRenderedAction() noexcept {
    state_->renderedActions.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordRedirect() noexcept {
    state_->redirects.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionRequest() noexcept {
    state_->interactionRequests.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionGetRequest() noexcept {
    state_->interactionGetRequests.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionPostRequest() noexcept {
    state_->interactionPostRequests.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionSuccessfulResponse() noexcept {
    state_->interactionSuccessfulResponses.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionClientErrorResponse() noexcept {
    state_->interactionClientErrorResponses.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordInteractionRedirect() noexcept {
    state_->interactionRedirects.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordComponentRendered(std::size_t depth) noexcept {
    state_->componentsRendered.fetch_add(1, std::memory_order_relaxed);
    updateMaximum(state_->maximumRenderDepth, depth);
}

void RuntimeDiagnostics::recordRenderFailure() noexcept {
    state_->renderFailures.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordServiceScopeResolution() noexcept {
    state_->serviceScopeResolutions.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordServiceScopeCacheHit() noexcept {
    state_->serviceScopeCacheHits.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordServiceScopeServiceCreated() noexcept {
    state_->serviceScopeServicesCreated.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSubmitted() noexcept {
    const auto queued = state_->jobQueued.fetch_add(1, std::memory_order_relaxed) + 1;
    updateMaximum(state_->jobPeakQueued, queued);
    state_->jobSubmitted.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobStarted() noexcept {
    state_->jobQueued.fetch_sub(1, std::memory_order_relaxed);
    const auto running = state_->jobRunning.fetch_add(1, std::memory_order_relaxed) + 1;
    updateMaximum(state_->jobPeakRunning, running);
    state_->jobStarted.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobCompleted() noexcept {
    state_->jobRunning.fetch_sub(1, std::memory_order_relaxed);
    state_->jobCompleted.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobFailed() noexcept {
    state_->jobRunning.fetch_sub(1, std::memory_order_relaxed);
    state_->jobFailed.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobCancelled(bool wasRunning) noexcept {
    (wasRunning ? state_->jobRunning : state_->jobQueued).fetch_sub(1, std::memory_order_relaxed);
    state_->jobCancelled.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobCancellationRequested() noexcept {
    state_->jobCancellationRequests.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobPauseRequested() noexcept {
    state_->jobPauseRequests.fetch_add(1, std::memory_order_relaxed);
}
void RuntimeDiagnostics::recordJobResumeRequested() noexcept {
    state_->jobResumeRequests.fetch_add(1, std::memory_order_relaxed);
}
void RuntimeDiagnostics::recordJobPaused() noexcept {
    state_->jobPaused.fetch_add(1, std::memory_order_relaxed);
}
void RuntimeDiagnostics::recordJobResumed(std::uint64_t milliseconds) noexcept {
    state_->jobPaused.fetch_sub(1, std::memory_order_relaxed);
    state_->jobPauseTotalMs.fetch_add(milliseconds, std::memory_order_relaxed);
    state_->jobPauseSamples.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobProgress() noexcept {
    state_->jobProgress.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobRejected() noexcept {
    state_->jobRejected.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::registerJobQueueCapacity(std::size_t count) noexcept {
    state_->jobQueueCapacity.fetch_add(count, std::memory_order_relaxed);
}
void RuntimeDiagnostics::unregisterJobQueueCapacity(std::size_t count) noexcept {
    state_->jobQueueCapacity.fetch_sub(count, std::memory_order_relaxed);
}

void RuntimeDiagnostics::registerJobWorkers(std::size_t count) noexcept {
    state_->jobWorkerCapacity.fetch_add(count, std::memory_order_relaxed);
}

void RuntimeDiagnostics::unregisterJobWorkers(std::size_t count) noexcept {
    state_->jobWorkerCapacity.fetch_sub(count, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobQueueWait(std::uint64_t value) noexcept {
    state_->jobQueueWaitTotalMs.fetch_add(value, std::memory_order_relaxed);
    state_->jobQueueWaitSamples.fetch_add(1, std::memory_order_relaxed);
    updateMaximum(state_->jobQueueWaitMaxMs, value);
}

void RuntimeDiagnostics::recordJobExecution(std::uint64_t value) noexcept {
    state_->jobExecutionTotalMs.fetch_add(value, std::memory_order_relaxed);
    state_->jobExecutionSamples.fetch_add(1, std::memory_order_relaxed);
    updateMaximum(state_->jobExecutionMaxMs, value);
}

void RuntimeDiagnostics::recordJobEventPublished() noexcept {
    state_->jobEventsPublished.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobEventGap() noexcept {
    state_->jobEventGaps.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSubscriptionOpened() noexcept {
    state_->jobSubscriptions.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSubscriptionClosed() noexcept {
    state_->jobSubscriptions.fetch_sub(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSseOpened() noexcept {
    state_->jobSseConnections.fetch_add(1, std::memory_order_relaxed);
    state_->jobSseOpened.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSseRejected() noexcept {
    state_->jobSseRejected.fetch_add(1, std::memory_order_relaxed);
}

void RuntimeDiagnostics::recordJobSseClosed() noexcept {
    state_->jobSseConnections.fetch_sub(1, std::memory_order_relaxed);
    state_->jobSseClosed.fetch_add(1, std::memory_order_relaxed);
}

RuntimeDiagnosticsSnapshot RuntimeDiagnostics::snapshot() const noexcept {
    RuntimeDiagnosticsSnapshot result;
#define LOAD_JOB(name, member) result.jobs.name = state_->member.load(std::memory_order_relaxed)
    LOAD_JOB(submitted, jobSubmitted);
    LOAD_JOB(started, jobStarted);
    LOAD_JOB(completed, jobCompleted);
    LOAD_JOB(failed, jobFailed);
    LOAD_JOB(cancelled, jobCancelled);
    LOAD_JOB(cancellationRequests, jobCancellationRequests);
    LOAD_JOB(paused, jobPaused);
    LOAD_JOB(pauseRequests, jobPauseRequests);
    LOAD_JOB(resumeRequests, jobResumeRequests);
    LOAD_JOB(pauseTotalMs, jobPauseTotalMs);
    LOAD_JOB(pauseSamples, jobPauseSamples);
    if (result.jobs.pauseSamples != 0) {
        result.jobs.pauseAverageMs = result.jobs.pauseTotalMs / result.jobs.pauseSamples;
    }
    LOAD_JOB(progressUpdates, jobProgress);
    LOAD_JOB(rejected, jobRejected);
    LOAD_JOB(queued, jobQueued);
    LOAD_JOB(running, jobRunning);
    LOAD_JOB(workerCapacity, jobWorkerCapacity);
    LOAD_JOB(queueCapacity, jobQueueCapacity);
    LOAD_JOB(peakQueued, jobPeakQueued);
    LOAD_JOB(peakRunning, jobPeakRunning);
    LOAD_JOB(queueWaitMaxMs, jobQueueWaitMaxMs);
    LOAD_JOB(executionMaxMs, jobExecutionMaxMs);
    LOAD_JOB(queueWaitSamples, jobQueueWaitSamples);
    LOAD_JOB(executionSamples, jobExecutionSamples);
    if (result.jobs.workerCapacity != 0) {
        result.jobs.workerUtilizationBps =
            result.jobs.running * 10000 / result.jobs.workerCapacity;
    }
    if (result.jobs.queueCapacity != 0) {
        result.jobs.queueSaturationBps =
            result.jobs.queued * 10000 / result.jobs.queueCapacity;
    }
    LOAD_JOB(queueWaitTotalMs, jobQueueWaitTotalMs);
    LOAD_JOB(executionTotalMs, jobExecutionTotalMs);
    if (result.jobs.queueWaitSamples != 0) {
        result.jobs.queueWaitAverageMs = result.jobs.queueWaitTotalMs / result.jobs.queueWaitSamples;
    }
    if (result.jobs.executionSamples != 0) {
        result.jobs.executionAverageMs = result.jobs.executionTotalMs / result.jobs.executionSamples;
    }
    LOAD_JOB(eventsPublished, jobEventsPublished);
    LOAD_JOB(eventGaps, jobEventGaps);
    LOAD_JOB(activeSubscriptions, jobSubscriptions);
    LOAD_JOB(activeSseConnections, jobSseConnections);
    LOAD_JOB(sseConnectionsOpened, jobSseOpened);
    LOAD_JOB(sseConnectionsRejected, jobSseRejected);
    LOAD_JOB(sseConnectionsClosed, jobSseClosed);
#undef LOAD_JOB

    result.requests.requests = state_->requests.load(std::memory_order_relaxed);
    result.requests.actions = state_->actions.load(std::memory_order_relaxed);
    result.requests.renderedActions = state_->renderedActions.load(std::memory_order_relaxed);
    result.requests.redirects = state_->redirects.load(std::memory_order_relaxed);

    result.interactions.requests = state_->interactionRequests.load(std::memory_order_relaxed);
    result.interactions.getRequests = state_->interactionGetRequests.load(std::memory_order_relaxed);
    result.interactions.postRequests = state_->interactionPostRequests.load(std::memory_order_relaxed);
    result.interactions.successfulResponses = state_->interactionSuccessfulResponses.load(std::memory_order_relaxed);
    result.interactions.clientErrorResponses = state_->interactionClientErrorResponses.load(std::memory_order_relaxed);
    result.interactions.redirects = state_->interactionRedirects.load(std::memory_order_relaxed);

    result.rendering.componentsRendered = state_->componentsRendered.load(std::memory_order_relaxed);
    result.rendering.renderFailures = state_->renderFailures.load(std::memory_order_relaxed);
    result.rendering.maximumDepth = state_->maximumRenderDepth.load(std::memory_order_relaxed);

    result.serviceScopes.resolutions = state_->serviceScopeResolutions.load(std::memory_order_relaxed);
    result.serviceScopes.cacheHits = state_->serviceScopeCacheHits.load(std::memory_order_relaxed);
    result.serviceScopes.servicesCreated = state_->serviceScopeServicesCreated.load(std::memory_order_relaxed);

    return result;
}

} // namespace drogular