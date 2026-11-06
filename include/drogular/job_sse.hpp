#pragma once

#include <drogular/job_events.hpp>

#include <string>
#include <string_view>

namespace drogular {

/** Transport-independent SSE framing for a JobEvent.
 * The HTTP adapter is responsible for auth, content type, flushing and
 * keeping the stream alive. Never serializes opaque std::any results.
 */
std::string encodeJobSse(const JobEvent& event);
// Used when a subscriber reports a replay/overflow gap.
std::string encodeJobSseReset();
std::string encodeJobSseHeartbeat();

} // namespace drogular