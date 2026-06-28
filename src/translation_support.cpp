#include <drogular/translation_support.hpp>

#include <drogular/locale_support.hpp>
#include <drogular/translation_provider.hpp>

namespace drogular {

std::string TranslationSupport::translate(
    RenderContext& context,
    const std::string& key
) {
    return translate(
        context,
        LocaleSupport::current(context),
        key
    );
}

std::string TranslationSupport::translate(
    RenderContext& context,
    const std::string& locale,
    const std::string& key
) {
    auto provider =
        context.service<TranslationProvider>();

    if (provider == nullptr) {
        return key;
    }

    return provider->translate(
        locale,
        key
    );
}

} // namespace drogular