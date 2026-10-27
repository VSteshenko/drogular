#pragma once

#include <drogular/locale_support.hpp>
#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

class MiniStoreStorefrontPageSupport {
public:
    static void apply(drogular::RenderContext& context, const std::string& titleKey) {
        drogular::PageSupport::apply(context, context.translate(titleKey));
        const auto locale = drogular::LocaleSupport::current(context);
        context.set("locale", locale);
        context.set("languageEnClass",
                    locale == "en" ? std::string(" is-active") : std::string{});
        context.set("languageDeClass",
                    locale == "de" ? std::string(" is-active") : std::string{});
        context.set("pageClass", std::string{});
        const auto request = context.request();
        context.set("languageRedirect", request
            ? request->path()
            : std::string("/"));
    }
};