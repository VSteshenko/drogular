#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/department_members/providers/department_member_provider.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <unordered_set>

class PortalDepartmentDetailsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "departments.details.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        const auto department =
            context.requireService<
                PortalDepartmentProvider
            >()->findById(id);

        context.set("departmentNotFound", !department.has_value());
        context.set("departmentsBackUrl", "/departments");

        if (!department) {
            return;
        }

        const auto users =
            context.requireService<PortalUserProvider>()->all();

        const auto memberships =
            context.requireService<
                PortalDepartmentMemberProvider
            >()->membersOfDepartment(id);

        const auto userById =
            [&users](int userId) -> std::optional<PortalUser> {
            for (const auto& user : users) {
                if (user.id == userId) {
                    return user;
                }
            }
            return std::nullopt;
        };

        context.set("departmentId", department->id);
        context.set("departmentName", department->name);
        context.set("departmentDescription", department->description);
        context.set(
            "departmentStatus",
            context.translate(
                department->isActive
                ? "departments.active"
                : "departments.inactive"
            )
        );
        const auto manager = userById(department->managerId);
        context.set(
            "departmentManager",
            manager
            ? manager->username
            : "#" + std::to_string(department->managerId)
        );

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
                std::to_string(id) +
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
        context.set("hasDepartmentMemberCandidates", !candidates.empty());
    }

    std::string templatePath() const override {
        return "department_details.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};