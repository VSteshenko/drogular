#pragma once

#include "portal_component_context.hpp"

#include <drogular/component.hpp>

class PortalPaginationComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "PortalPagination";

    void onInit(drogular::RenderContext& context) override {
        const auto modelPath =
            input<std::string>("model").value_or("");

        context.set(
            "paginationModel",
            portal::ui::resolveJson(context, modelPath)
        );
    }

    std::string templatePath() const override {
        return "components/portal_pagination.html";
    }
};