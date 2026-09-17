#pragma once

#include "portal_component_context.hpp"

#include <drogular/component.hpp>

#include <algorithm>

class PortalPaginationComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "PortalPagination";

    void onInit(drogular::RenderContext& context) override {
        const auto modelPath = input<std::string>("model").value_or("");
        auto model = portal::ui::resolveJson(context, modelPath);
        const auto submitName = input<std::string>("submitName").value_or("");

        if (!submitName.empty() && model.isMember("pages")) {
            int currentPage = 1;
            for (const auto& page : model["pages"]) {
                if (page.get("current", false).asBool()) {
                    currentPage = page.get("number", 1).asInt();
                    break;
                }
            }
            model["previousPage"] = std::max(1, currentPage - 1);
            model["nextPage"] = currentPage + 1;
        }

        context.set("paginationModel", model);
        context.set("paginationSubmitName", submitName);
        context.set("paginationUsesSubmit", !submitName.empty());
    }

    std::string templatePath() const override {
        return "components/portal_pagination.html";
    }
};