#pragma once

#include "features/localization/support/portal_error_translator.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/render_context.hpp>

class PortalDepartmentCreateFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& name = {},
        const std::string& description = {},
        const std::string& managerId = {},
        bool isActive = true,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto departmentsError =
            PortalErrorTranslator::departmentsError(context, error);
        const auto departmentsSuccess =
            PortalErrorTranslator::departmentsSuccess(context, success);

        context.set("hasDepartmentsError", !departmentsError.empty());
        context.set("hasDepartmentsSuccess", !departmentsSuccess.empty());
        context.set(
            "alertMessage",
            !departmentsError.empty()
                ? departmentsError
                : departmentsSuccess
        );

        context.set("createDepartmentName", name);
        context.set("createDepartmentDescription", description);
        context.set("createDepartmentActive", isActive);

        const auto users =
            context.requireService<PortalUserProvider>()->all();

        Json::Value managers(Json::arrayValue);
        for (const auto& user : users) {
            Json::Value item(Json::objectValue);
            item["id"] = user.id;
            item["name"] = user.username;
            item["selected"] =
                managerId == std::to_string(user.id);
            managers.append(std::move(item));
        }
        context.set("departmentManagers", managers);
    }
};