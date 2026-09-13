#pragma once

#include <string_view>

namespace drogular::ui_resources {

inline constexpr std::string_view StylesheetPath =
    "/__drogular/assets/ui.css";

std::string_view stylesheet();

} // namespace drogular::ui_resources