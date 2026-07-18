#pragma once

#include "data/portal_schema.hpp"
#include "localization/portal_error_translator.hpp"
#include "providers/project_type_provider.hpp"
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

        const auto projectTypesError =
            PortalErrorTranslator::projectTypesError(
                context,
                error
            );

        const auto schema =
            PortalSchema::projectTypes();

        context.set(
            "projectTypeCodeLabel",
            context.translate(
                schema.fieldLabelKey("code")
            )
        );

        context.set(
            "projectTypeTitleLabel",
            context.translate(
                schema.fieldLabelKey("title")
            )
        );

        context.set(
            "projectTypeCodeRequired",
            schema.fieldRequired("code")
        );

        context.set(
            "projectTypeTitleRequired",
            schema.fieldRequired("title")
        );

        context.set(
            "hasProjectTypesError",
            !projectTypesError.empty()
        );

        context.set(
            "alertMessage",
            projectTypesError
        );

        context.set(
            "projectTypeId",
            projectType->id
        );

        context.set(
            "projectTypeCode",
            projectType->code
        );

        context.set(
            "projectTypeTitle",
            projectType->title
        );
    }

    std::string templatePath() const override {
        return "project_type_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};