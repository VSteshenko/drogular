#pragma once

#include "ui/portal_page_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_query_parser.hpp"
#include "features/departments/ui/portal_department_query_serializer.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <algorithm>

class PortalDepartmentsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "departments.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto request =
            context.request();

        const auto query =
            PortalDepartmentQueryParser::fromRequest(request);

        auto provider =
            context.requireService<
                PortalDepartmentProvider
            >();

        auto users =
            context.requireService<
                PortalUserProvider
            >();

        const auto allUsers =
            users->all();

        Json::Value managers(Json::arrayValue);
        for (const auto& user : allUsers) {
            Json::Value item(Json::objectValue);

            item["id"] = user.id;
            item["name"] = user.username;

            managers.append(std::move(item));
        }

        context.set(
            "departmentManagers",
            managers);

        context.set(
            "departmentSearch",
            query.search.value_or(""));

        Json::Value activeOptions(Json::arrayValue);
        const auto addActive =
            [&activeOptions, &query](
                const std::string& value,
                const std::string& label,
                std::optional<bool> state) {
                Json::Value item(Json::objectValue);

                item["value"] = value;
                item["label"] = label;
                item["selected"] =
                    query.isActive == state;

                activeOptions.append(std::move(item));
        };

        addActive(
            "",
            context.translate("departments.filter.all"),
            std::nullopt
        );
        addActive(
            "true",
            context.translate("departments.filter.active"),
            true
        );
        addActive(
            "false",
            context.translate("departments.filter.inactive"),
            false
        );

        context.set(
            "departmentActiveOptions",
            activeOptions
        );

        const auto sort =
            query.sorting.empty()
            ? PortalDepartmentSort{}
            : query.sorting.front();

        Json::Value sortOptions(Json::arrayValue);
        for (const auto& entry :
            std::vector<std::pair<std::string,std::string>>{
                {"name","departments.sort.name"},
                {"managerId","departments.sort.manager"},
                {"isActive","departments.sort.active"},
                {"id","departments.sort.id"}
            }) {
            Json::Value item(Json::objectValue);

            item["value"] = entry.first;
            item["label"] = context.translate(entry.second);
            item["selected"] =
                sort.field == entry.first;

            sortOptions.append(std::move(item));
        }

        context.set(
            "departmentSortOptions",
            sortOptions
        );

        Json::Value directions(Json::arrayValue);
        for (const auto& entry :
            std::vector<std::pair<std::string,std::string>>{
                {"asc","common.ascending"},
                {"desc","common.descending"}
            }) {
            Json::Value item(Json::objectValue);

            item["value"] = entry.first;
            item["label"] = context.translate(entry.second);
            item["selected"] =
                toString(sort.direction) == entry.first;

            directions.append(std::move(item));
        }

        context.set(
            "departmentDirectionOptions",
            directions);

        const auto result =
            provider->search(query);

        const auto managerName =
            [&allUsers](int id) {
                for (const auto& user : allUsers) {
                    if (user.id == id) {
                        return user.username;
                    }
                }

                return std::string("#") + std::to_string(id);
            };

        const auto pageUrl =
            [&query](int page) {
                auto value = query;

                value.page = std::max(1, page);

                return std::string("/departments") +
                    PortalDepartmentQuerySerializer::toQueryString(value);
            };

        Json::Value departments(Json::arrayValue);
        for (const auto& value : result.items) {
            Json::Value item(Json::objectValue);

            item["id"] = value.id;
            item["name"] = value.name; item["description"] = value.description; item["manager"] = managerName(value.managerId); item["active"] = value.isActive; item["status"] = context.translate(value.isActive ? "departments.active" : "departments.inactive"); item["editUrl"] = "/departments/" + std::to_string(value.id) + "/edit"; departments.append(std::move(item));
        }

        context.set(
            "departments",
            departments
        );
        context.set(
            "hasDepartments",
            !departments.empty()
        );
        context.set(
            "departmentTotalItems",
            result.totalItems
        );

        Json::Value pages(Json::arrayValue);
        for (int page = 1; page <= result.totalPages; ++page) {
            Json::Value item(Json::objectValue);

            item["number"] = page;
            item["url"] = pageUrl(page);
            item["current"] =
                page == result.page;

            pages.append(std::move(item));
        }

        context.set(
            "hasDepartmentPagination",
            result.totalPages > 1
        );
        context.set(
            "departmentPaginationPages",
            pages
        );
        context.set(
            "hasPreviousDepartmentPage",
            result.page > 1
        );
        context.set(
            "previousDepartmentPageUrl",
            pageUrl(result.page - 1)
        );
        context.set(
            "hasNextDepartmentPage",
            result.page < result.totalPages
        );
        context.set(
            "nextDepartmentPageUrl",
            pageUrl(result.page + 1)
        );
    }

    std::string templatePath() const override {
        return "departments.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};