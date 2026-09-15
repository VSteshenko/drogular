#pragma once

#include <string_view>

namespace drogular::ui_resources {

inline constexpr std::string_view StylesheetPath =
    "/__drogular/assets/ui.css";

inline constexpr std::string_view ScriptPath =
    "/__drogular/assets/ui.js";

std::string_view stylesheet();
std::string_view script();

} // namespace drogular::ui_resources