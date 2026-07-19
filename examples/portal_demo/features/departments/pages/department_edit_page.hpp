#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>

class PortalDepartmentEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "departments.edit.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
            context,
            "role",
            "admin")
        ) {
            return;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        auto provider =
            context.requireService<
                PortalDepartmentProvider
            >();

        const auto department =
            provider->findById(id);

        context.set(
            "departmentNotFound",
            !department.has_value()
        );

        if (!department) {
            return;
        }

        Json::Value managers(Json::arrayValue);
        for (const auto& user :
            context.requireService<PortalUserProvider>()->all()) {
            Json::Value item(Json::objectValue);

            item["id"] = user.id;
            item["name"] = user.username;
            item["selected"] =
                user.id == department->managerId;

            managers.append(std::move(item));
        }
        context.set("departmentManagers", managers);
        context.set("departmentId", department->id);
        context.set("departmentName", department->name);
        context.set("departmentDescription", department->description);
        context.set("departmentActive", department->isActive);
    }

    std::string templatePath() const override {
        return "department_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};