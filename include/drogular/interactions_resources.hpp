#pragma once

#include <string_view>

namespace drogular::interactions_resources {

inline constexpr std::string_view ScriptPath =
    "/__drogular/assets/interactions.js";

std::string_view script();

} // namespace drogular::interactions_resources