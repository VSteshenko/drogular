#pragma once

#include <drogular/developer_tools/application_inspection.hpp>
#include <drogular/runtime_diagnostics.hpp>
#include <drogular/process_metrics.hpp>
#include <drogular/process_metrics_history.hpp>

namespace drogular {

/**
 * Exposes aggregate runtime diagnostics through ApplicationInspection.
 *
 * The contributor is intentionally a read-only adapter: runtime subsystems
 * record counters without depending on inspection or Developer Tools, while
 * this layer converts an immutable snapshot into inspection sections.
 */
class RuntimeDiagnosticsContributor final
    : public DeveloperToolsContributor {
public:
    explicit RuntimeDiagnosticsContributor(
        const RuntimeDiagnostics& diagnostics,
        const ProcessMetricsProvider* processMetrics = nullptr,
        const ProcessMetricsHistory* history = nullptr
    ) noexcept;

    void contribute(
        ApplicationInspection& inspection
    ) const override;

private:
    const RuntimeDiagnostics& diagnostics_;
    const ProcessMetricsProvider* processMetrics_;
    const ProcessMetricsHistory* history_;
};

} // namespace drogular