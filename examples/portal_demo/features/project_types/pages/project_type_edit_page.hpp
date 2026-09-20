#pragma once

#include "features/project_types/ui/portal_project_type_edit_form_support.hpp"
#include "features/project_types/providers/project_type_provider.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalProjectTypeEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "project_types.edit_page.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(
                context
            )) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
        }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        const auto projectType =
            projectTypes->findById(id);

        context.set(
            "projectTypeNotFound",
            !projectType.has_value()
        );

        if (!projectType.has_value()) {
            return;
        }

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        PortalProjectTypeEditFormSupport::apply(
            context,
            projectType->id,
            projectType->code,
            projectType->title,
            error,
            success
        );
    }

    std::string templatePath() const override {
        return "project_type_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};