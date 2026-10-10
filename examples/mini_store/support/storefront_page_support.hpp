#pragma once

#include <drogular/locale_support.hpp>
#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

class MiniStoreStorefrontPageSupport {
public:
    static void apply(drogular::RenderContext& context, const std::string& titleKey) {
        drogular::PageSupport::apply(context, context.translate(titleKey));
        context.set("locale", drogular::LocaleSupport::current(context));
        const auto request = context.request();
        context.set("languageRedirect", request
            ? request->path()
            : std::string("/"));
    }
};