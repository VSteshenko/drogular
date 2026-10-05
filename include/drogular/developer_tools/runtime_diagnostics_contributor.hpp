#pragma once

#include <drogular/developer_tools/application_inspection.hpp>
#include <drogular/runtime_diagnostics.hpp>

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
        const RuntimeDiagnostics& diagnostics
    ) noexcept;

    void contribute(
        ApplicationInspection& inspection
    ) const override;

private:
    const RuntimeDiagnostics& diagnostics_;
};

} // namespace drogular