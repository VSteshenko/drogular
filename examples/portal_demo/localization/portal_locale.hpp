#pragma once

#include <drogular/action_context.hpp>
#include <drogular/component.hpp>

#include <string>

class PortalLocale {
public:
    static std::string fromRenderContext(
        drogular::RenderContext& context
    ) {
        const auto lang =
            context.cookie("lang");

        if (lang.has_value()) {
            return normalize(*lang);
        }

        return "en";
    }

    static std::string fromActionContext(
        drogular::ActionContext& context
    ) {
        const auto lang =
            context.cookie("lang");

        if (lang.has_value()) {
            return normalize(*lang);
        }

        return "en";
    }

private:
    static std::string normalize(
        const std::string& value
    ) {
        if (value == "de") {
            return "de";
        }

        return "en";
    }
};