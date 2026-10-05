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

struct RuntimeDiagnosticsSnapshot {
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

    RuntimeDiagnosticsSnapshot snapshot() const noexcept;

private:
    struct State;
    State* state_;
};

} // namespace drogular