#include <drogular/runtime_diagnostics.hpp>

#include <atomic>

namespace drogular {

namespace {

void updateMaximum(
    std::atomic<std::size_t>& maximum,
    std::size_t value
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

RuntimeDiagnosticsSnapshot RuntimeDiagnostics::snapshot() const noexcept {
    RuntimeDiagnosticsSnapshot result;

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