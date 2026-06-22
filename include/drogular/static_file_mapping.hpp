#pragma once

#include <filesystem>
#include <string>

namespace drogular {

struct StaticFileMapping {
    std::string routePrefix;
    std::filesystem::path directory;
};

} // namespace drogular