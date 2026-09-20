#pragma once

#include "data/portal_schema.hpp"
#include "features/localization/support/portal_error_translator.hpp"

#include <drogular/render_context.hpp>

#include <string>

class PortalProjectTypeEditFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        int projectTypeId,
        const std::string& code,
        const std::string& title,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto projectTypesError =
            PortalErrorTranslator::projectTypesError(context, error);
        const auto projectTypesSuccess =
            PortalErrorTranslator::projectTypesSuccess(context, success);

        context.set("hasProjectTypesError", !projectTypesError.empty());
        context.set("hasProjectTypesSuccess", !projectTypesSuccess.empty());
        context.set(
            "alertMessage",
            !projectTypesError.empty()
                ? projectTypesError
                : projectTypesSuccess
        );

        const auto schema = PortalSchema::projectTypes();

        context.set("projectTypeId", projectTypeId);
        context.set("projectTypeCode", code);
        context.set("projectTypeTitle", title);
        context.set(
            "projectTypeCodeLabel",
            context.translate(schema.fieldLabelKey("code"))
        );
        context.set(
            "projectTypeTitleLabel",
            context.translate(schema.fieldLabelKey("title"))
        );
        context.set(
            "projectTypeCodeRequired",
            schema.fieldRequired("code")
        );
        context.set(
            "projectTypeTitleRequired",
            schema.fieldRequired("title")
        );
    }
};