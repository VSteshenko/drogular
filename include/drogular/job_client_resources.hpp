#pragma once

#include <string_view>

namespace drogular::job_client_resources {

// Public, opt-in ES module. No browser DOM or application-specific endpoints.
inline constexpr std::string_view ScriptPath =
    "/__drogular/assets/job-client.js";

std::string_view script();

} // namespace drogular::job_client_resources