#pragma once

#include <string_view>

namespace drogular::localization_resources {

inline constexpr std::string_view ScriptPath =
    "/__drogular/assets/localization.js";

std::string_view script();

} // namespace drogular::localization_resources