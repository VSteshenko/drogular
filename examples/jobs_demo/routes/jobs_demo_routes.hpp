#pragma once

#include <memory>

namespace drogular {
class JobRuntime;
class RuntimeDiagnostics;
}

namespace jobsdemo {
void installRoutes(std::shared_ptr<drogular::JobRuntime> jobs,
                   drogular::RuntimeDiagnostics& diagnostics);
}
