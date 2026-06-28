#include <drogular/locale_support.hpp>

namespace drogular {

std::string LocaleSupport::current(
    RenderContext& context
) {
    return current(context, "en");
}

std::string LocaleSupport::current(
    RenderContext& context,
    const std::string& defaultLocale
) {
    const auto locale =
        context.cookie("lang");

    if (!locale.has_value()) {
        return defaultLocale;
    }

    if (*locale == "de") {
        return "de";
    }

    if (*locale == "en") {
        return "en";
    }

    return defaultLocale;
}

} // namespace drogular