#pragma once

#include <memory>

namespace drogular {

class JobRuntime;
class JobRegistry;
class RuntimeDiagnostics;

}

namespace jobsdemo {

void installRoutes(std::shared_ptr<drogular::JobRuntime> jobs,
                   std::shared_ptr<drogular::JobRegistry> registry,
                   drogular::RuntimeDiagnostics& diagnostics);

}