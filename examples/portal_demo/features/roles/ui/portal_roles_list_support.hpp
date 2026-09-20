#pragma once

#include "features/localization/support/portal_error_translator.hpp"
#include "features/roles/providers/role_provider.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/render_context.hpp>

#include <string>
#include <unordered_map>

class PortalRolesListSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto rolesError =
            PortalErrorTranslator::rolesError(context, error);
        const auto rolesSuccess =
            PortalErrorTranslator::rolesSuccess(context, success);

        context.set("hasRolesListError", !rolesError.empty());
        context.set("hasRolesListSuccess", !rolesSuccess.empty());
        context.set(
            "rolesListAlertMessage",
            !rolesError.empty() ? rolesError : rolesSuccess
        );

        auto roles =
            context.requireService<PortalRoleProvider>();
        auto users =
            context.requireService<PortalUserProvider>();

        std::unordered_map<std::string, int> userCounts;
        for (const auto& user : users->all()) {
            ++userCounts[user.role];
        }

        Json::Value items(Json::arrayValue);
        for (const auto& role : roles->all()) {
            const auto found = userCounts.find(role.code);
            const auto userCount =
                found != userCounts.end()
                    ? found->second
                    : 0;

            Json::Value item(Json::objectValue);
            item["title"] = role.title;
            item["subtitle"] = role.code;
            item["count"] = userCount;
            item["countLabel"] = context.translate("roles.users");
            item["canDelete"] = userCount == 0;
            item["editUrl"] =
                "/roles/" + std::to_string(role.id) + "/edit";
            item["deleteUrl"] =
                "/roles/" + std::to_string(role.id) + "/delete";
            items.append(std::move(item));
        }

        context.set("roles", items);
    }
};