#pragma once

#include "features/department_members/providers/department_member_provider.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/render_context.hpp>

#include <optional>
#include <string>
#include <unordered_set>

class PortalDepartmentMembersSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        int departmentId,
        const std::string& returnUrl,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto memberError =
            PortalErrorTranslator::departmentMembersError(context, error);
        const auto memberSuccess =
            PortalErrorTranslator::departmentMembersSuccess(context, success);

        context.set("hasDepartmentMembersError", !memberError.empty());
        context.set("hasDepartmentMembersSuccess", !memberSuccess.empty());
        context.set(
            "departmentMembersAlertMessage",
            !memberError.empty() ? memberError : memberSuccess
        );
        context.set("departmentMembersReturnUrl", returnUrl);

        const auto users =
            context.requireService<PortalUserProvider>()->all();
        const auto memberships =
            context.requireService<
                PortalDepartmentMemberProvider
            >()->membersOfDepartment(departmentId);

        const auto userById =
            [&users](int userId) -> std::optional<PortalUser> {
            for (const auto& user : users) {
                if (user.id == userId) {
                    return user;
                }
            }
            return std::nullopt;
        };

        std::unordered_set<int> memberIds;
        Json::Value members(Json::arrayValue);
        for (const auto& membership : memberships) {
            memberIds.insert(membership.userId);

            const auto user = userById(membership.userId);
            if (!user) {
                continue;
            }

            Json::Value item(Json::objectValue);
            item["userId"] = user->id;
            item["username"] = user->username;
            item["role"] = user->role;
            item["removeUrl"] =
                "/departments/" +
                std::to_string(departmentId) +
                "/members/" +
                std::to_string(user->id) +
                "/remove";
            members.append(std::move(item));
        }
        context.set("departmentMembers", members);

        Json::Value candidates(Json::arrayValue);
        for (const auto& user : users) {
            if (memberIds.contains(user.id)) {
                continue;
            }

            Json::Value item(Json::objectValue);
            item["id"] = user.id;
            item["name"] = user.username;
            candidates.append(std::move(item));
        }
        context.set("departmentMemberCandidates", candidates);
    }
};