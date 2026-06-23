#include <drogular/static_file_etag.hpp>

namespace drogular {

std::string StaticFileEtag::create(
    const std::filesystem::path& path
) {
    const auto size =
        std::filesystem::file_size(path);

    const auto lastWriteTime =
        std::filesystem::last_write_time(path)
            .time_since_epoch()
            .count();

    return "W/\"" +
           std::to_string(
               static_cast<long long>(size)
           ) +
           "-" +
           std::to_string(
               static_cast<long long>(lastWriteTime)
           ) +
           "\"";
}

} // namespace drogular
