#pragma once

#include "features/departments/data/portal_department.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/render_context.hpp>

class PortalDepartmentEditFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const PortalDepartment& department,
        const std::string& name,
        const std::string& description,
        int managerId,
        bool isActive,
        const std::string& error = {},
        const std::string& success = {},
        const std::string& returnUrl = "/departments"
    ) {
        const auto translatedError =
            PortalErrorTranslator::departmentsError(context, error);
        const auto translatedSuccess =
            PortalErrorTranslator::departmentsSuccess(context, success);
        context.set("hasDepartmentsError", !translatedError.empty());
        context.set("hasDepartmentsSuccess", !translatedSuccess.empty());
        context.set("alertMessage", !translatedError.empty()
            ? translatedError
            : translatedSuccess);

        const auto safeReturnUrl =
            PortalDepartmentNavigationSupport::departmentsReturnUrl(returnUrl);
        context.set("departmentId", department.id);
        context.set("departmentName", name);
        context.set("departmentDescription", description);
        context.set("departmentActive", isActive);
        context.set("departmentsReturnUrl", safeReturnUrl);
        context.set("departmentDetailsUrl",
            PortalDepartmentNavigationSupport::detailsUrl(department.id, safeReturnUrl));

        Json::Value managers(Json::arrayValue);
        for (const auto& user : context.requireService<PortalUserProvider>()->all()) {
            Json::Value item(Json::objectValue);
            item["id"] = user.id;
            item["name"] = user.username;
            item["selected"] = user.id == managerId;
            managers.append(std::move(item));
        }
        context.set("departmentManagers", managers);
    }
};