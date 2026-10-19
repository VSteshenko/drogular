#pragma once

#include <drogular/locale_support.hpp>
#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

class MiniStoreStaffPageSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& titleKey,
        const std::string& languageRedirect
    ) {
        drogular::PageSupport::apply(context, context.translate(titleKey));
        context.set("locale", drogular::LocaleSupport::current(context));
        context.set("languageRedirect", languageRedirect);
    }
};