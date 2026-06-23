#include <drogular/static_file_last_modified.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace drogular {

std::string StaticFileLastModified::create(
    const std::filesystem::path& path
) {
    const auto fileTime =
        std::filesystem::last_write_time(path);

    const auto systemTime =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            fileTime - std::filesystem::file_time_type::clock::now()
            + std::chrono::system_clock::now()
        );

    const auto time =
        std::chrono::system_clock::to_time_t(systemTime);

    std::tm gmt{};

#if defined(_WIN32)
    gmtime_s(&gmt, &time);
#else
    gmtime_r(&time, &gmt);
#endif

    std::ostringstream stream;

    stream << std::put_time(
        &gmt,
        "%a, %d %b %Y %H:%M:%S GMT"
    );

    return stream.str();
}

bool StaticFileLastModified::matches(
    const std::filesystem::path& path,
    const std::string& requestValue
) {
    return !requestValue.empty() &&
           requestValue == create(path);
}

} // namespace drogular