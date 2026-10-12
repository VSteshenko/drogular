#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace mini_store_time {

using Clock = std::chrono::system_clock;

inline std::string formatUtc(Clock::time_point timePoint) {
    const auto time = Clock::to_time_t(timePoint);
    std::tm utc{};

#if defined(_WIN32)
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif

    std::ostringstream stream;
    stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

inline std::string now() {
    return formatUtc(Clock::now());
}

inline std::string ago(Clock::duration duration) {
    return formatUtc(Clock::now() - duration);
}

} // namespace mini_store_time