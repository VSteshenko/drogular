#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/client_localization_catalog.hpp>
#include <drogular/file_translation_provider.hpp>
#include <drogular/translation_provider.hpp>

#include <drogon/HttpTypes.h>

#include <memory>

namespace portal_localization_detail {

inline drogular::ClientLocalizationCatalog catalog(
    drogular::ActionContext& context
) {
    const auto provider = context.requireService<drogular::TranslationProvider>();
    const auto fileProvider =
        std::dynamic_pointer_cast<drogular::FileTranslationProvider>(provider);

    if (fileProvider == nullptr) {
        throw drogular::ActionContextError(
            "Portal client localization requires FileTranslationProvider"
        );
    }

    return drogular::ClientLocalizationCatalog(
        fileProvider->catalog(),
        fileProvider->defaultLocale(),
        "/assets/localization",
        ""
    );
}

} // namespace portal_localization_detail

class PortalLocalizationManifestAction final
    : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        return drogular::ActionResult::json(
            portal_localization_detail::catalog(context).manifest().toJson()
        );
    }
};

class PortalLocalizationResourceAction final
    : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto locale = context.requireRouteParam("locale");
        const auto clientCatalog = portal_localization_detail::catalog(context);

        if (!clientCatalog.hasLocale(locale)) {
            return drogular::ActionResult::html(
                "Unknown localization locale",
                drogon::k404NotFound
            );
        }

        return drogular::ActionResult::json(
            clientCatalog.resource(locale).toJson()
        );
    }
};