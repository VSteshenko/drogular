#pragma once

#include "portal_component_context.hpp"

#include <drogular/component.hpp>

class PortalSelectComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "PortalSelect";

    void onInit(drogular::RenderContext& context) override {
        const auto optionsPath =
            input<std::string>("options").value_or("");
        const auto selectId =
            input<std::string>("id").value_or("");

        context.set("selectOptions", portal::ui::resolveJson(context, optionsPath));
        context.set("selectId", selectId);
        context.set("hasSelectId", !selectId.empty());
        context.set(
            "selectRequired",
            portal::ui::inputFlag(*this, "required")
        );
        context.set(
            "selectDisabled",
            portal::ui::inputFlag(*this, "disabled")
        );
    }

    std::string templatePath() const override {
        return "components/portal_select.html";
    }
};