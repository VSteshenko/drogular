#pragma once

#include "../providers/project_provider.hpp"
#include "../providers/project_type_provider.hpp"
#include "../providers/user_provider.hpp"
#include "../ui/portal_page_support.hpp"
#include "../data/portal_schema.hpp"
#include "../localization/portal_error_translator.hpp"
#include "../ui/models/portal_project_filter_view_model.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <algorithm>

class PortalProjectsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "projects.title"
        );

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

        const auto createTitle =
            request != nullptr
                ? request->getParameter("title")
                : std::string("");

        const auto projectsError =
            PortalErrorTranslator::projectsError(
                context,
                error
            );

        const auto projectsSuccess =
            PortalErrorTranslator::projectsSuccess(
                context,
                success
            );

        context.set("hasProjectsError", !projectsError.empty());
        context.set("hasProjectsSuccess", !projectsSuccess.empty());
        context.set("alertMessage", !projectsError.empty() ? projectsError : projectsSuccess);
        context.set("createProjectTitle", createTitle);

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto schema =
            PortalSchema::projects();

        context.set(
            "projectTitleRequired",
            schema.fieldRequired("title")
        );

        context.set(
            "projectTypeRequired",
            schema.fieldRequired("projectTypeId")
        );

        context.set(
            "projectStatusRequired",
            schema.fieldRequired("status")
        );

        auto projectTypes =
            context.requireService<PortalProjectTypeProvider>();

        const auto allProjectTypes =
            projectTypes->all();

        Json::Value options(Json::arrayValue);

        for (const auto& type : allProjectTypes) {
            Json::Value option(Json::objectValue);

            option["value"] = type.id;
            option["label"] = type.title;

            options.append(
                std::move(option)
            );
        }

        context.set(
            "projectTypeOptions",
            options
        );

        context.set(
            "projectsTitleLabel",
            context.translate(schema.fieldLabelKey("title"))
        );
        context.set(
            "typeLabel",
            context.translate(schema.fieldLabelKey("projectTypeId"))
        );
        context.set(
            "projectsStatusLabel",
            context.translate(schema.fieldLabelKey("status"))
        );

        const auto search =
            request != nullptr
                ? request->getParameter("search")
                : std::string("");

        context.set("projectSearch", search);

        const auto status =
            request != nullptr
                ? request->getParameter("status")
                : std::string("");

        context.set("projectStatusFilter", status);

        const auto projectTypeIdValue =
            request != nullptr
                ? request->getParameter("projectTypeId")
                : std::string("");

        std::optional<int> projectTypeId;

        if (!projectTypeIdValue.empty()) {
            try {
                const auto value =
                    std::stoi(projectTypeIdValue);

                if (value > 0) {
                    projectTypeId = value;
                }
            } catch (const std::exception&) {
            }
        }

        const auto ownerIdValue =
            request != nullptr
                ? request->getParameter("ownerId")
                : std::string("");

        std::optional<int> ownerId;

        if (!ownerIdValue.empty()) {
            try {
                const auto value =
                    std::stoi(ownerIdValue);

                if (value > 0) {
                    ownerId = value;
                }
            } catch (const std::exception&) {
            }
        }

        Json::Value statusOptions(Json::arrayValue);

        const auto addStatusOption =
            [&statusOptions, &status](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] =
                    status == value;

                statusOptions.append(
                    std::move(option)
                );
        };

        addStatusOption(
            "",
            context.translate(
                "projects.filter.status.all"
            )
        );

        addStatusOption(
            "active",
            context.translate(
                "projects.status.active"
            )
        );

        addStatusOption(
            "paused",
            context.translate(
                "projects.status.paused"
            )
        );

        addStatusOption(
            "done",
            context.translate(
                "projects.status.done"
            )
        );


        Json::Value projectTypeFilterOptions(Json::arrayValue);

        {
            Json::Value option(Json::objectValue);

            option["value"] = "";
            option["label"] =
                context.translate(
                    "projects.filter.type.all"
                );

            option["selected"] =
                !projectTypeId.has_value();

            projectTypeFilterOptions.append(
                std::move(option)
            );
        }

        for (const auto& type : allProjectTypes) {
            Json::Value option(Json::objectValue);

            option["value"] = type.id;
            option["label"] = type.title;

            option["selected"] =
                projectTypeId.has_value() &&
                *projectTypeId == type.id;

            projectTypeFilterOptions.append(
                std::move(option)
            );
        }

        auto users =
            context.requireService<PortalUserProvider>();

        const auto allUsers =
            users->all();

        Json::Value ownerFilterOptions(Json::arrayValue);

        {
            Json::Value option(Json::objectValue);

            option["value"] = "";
            option["label"] =
                context.translate("projects.filter.owner.all");
            option["selected"] = !ownerId.has_value();

            ownerFilterOptions.append(
                std::move(option)
            );
        }

        for (const auto& user : allUsers) {
            Json::Value option(Json::objectValue);

            option["value"] = user.id;
            option["label"] = user.username;
            option["selected"] =
                ownerId.has_value() &&
                *ownerId == user.id;

            ownerFilterOptions.append(
                std::move(option)
            );
        }

        const auto sortValue =
            request != nullptr
                ? request->getParameter("sort")
                : std::string("");

        const auto sortField =
            sortValue == "id" ||
            sortValue == "status" ||
            sortValue == "title"
                ? sortValue
                : std::string("title");

        Json::Value sortOptions(Json::arrayValue);

        const auto addSortOption =
            [&sortOptions, &sortField](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] =
                    sortField == value;

                sortOptions.append(
                    std::move(option)
                );
        };

        addSortOption(
            "title",
            context.translate(
                "projects.sort.title"
            )
        );

        addSortOption(
            "status",
            context.translate(
                "projects.sort.status"
            )
        );

        addSortOption(
            "id",
            context.translate(
                "projects.sort.id"
            )
        );

        const auto directionValue =
            request != nullptr
                ? request->getParameter("direction")
                : std::string("");

        const auto sortDirection =
            directionValue == "desc"
                ? PortalProjectSortDirection::
                      Descending
                : PortalProjectSortDirection::
                      Ascending;

        Json::Value directionOptions(Json::arrayValue);

        const auto selectedDirection =
            toString(sortDirection);

        const auto addDirectionOption =
            [&directionOptions,
             &selectedDirection](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] =
                    selectedDirection == value;

                directionOptions.append(
                    std::move(option)
                );
        };

        addDirectionOption(
            "asc",
            context.translate(
                "projects.sort.ascending"
            )
        );

        addDirectionOption(
            "desc",
            context.translate(
                "projects.sort.descending"
            )
        );

        portal::PortalProjectFilterViewModel filters;

        filters.search = search;
        filters.statusOptions = std::move(statusOptions);
        filters.projectTypeOptions = std::move(projectTypeFilterOptions);
        filters.ownerOptions = std::move(ownerFilterOptions);
        filters.sortOptions = std::move(sortOptions);
        filters.sortDirectionOptions = std::move(directionOptions);

        filters.hasActiveFilters =
            !search.empty() ||
            !status.empty() ||
            projectTypeId.has_value() ||
            ownerId.has_value();

        context.setJson("filters", filters);

        const auto pageValue =
            request != nullptr
                ? request->getParameter("page")
                : std::string();

        int requestedPage = 1;

        if (!pageValue.empty()) {
            try {
                requestedPage =
                    std::max(1, std::stoi(pageValue));
            } catch (const std::exception&) {
            }
        }

        auto repository =
            context.requireService<PortalProjectProvider>();

        PortalProjectFilter filter;
        filter.page = requestedPage;
        filter.pageSize = 10;

        std::string baseUrl = "/projects";
        std::string separator = "?";

        const auto appendParameter =
            [&baseUrl, &separator](
                const std::string& name,
                const std::string& value
            ) {
                baseUrl +=
                    separator +
                    name +
                    "=" +
                    drogular::Url::encode(value);
                separator = "&";
            };

        if (!search.empty()) {
            filter.search = search;
            appendParameter("search", search);
        }

        if (!status.empty()) {
            filter.status = status;
            appendParameter("status", status);
        }

        if (projectTypeId.has_value()) {
            filter.projectTypeId = *projectTypeId;
            appendParameter(
                "projectTypeId",
                std::to_string(*projectTypeId)
            );
        }

        if (ownerId.has_value()) {
            filter.ownerId = *ownerId;
            appendParameter(
                "ownerId",
                std::to_string(*ownerId)
            );
        }

        filter.sorting.push_back({
            .field = sortField,
            .direction = sortDirection
        });

        const auto hasCustomSorting =
            sortField != "title" ||
            sortDirection !=
                PortalProjectSortDirection::Ascending;

        if (hasCustomSorting) {
            appendParameter("sort", sortField);
            appendParameter(
                "direction",
                toString(sortDirection)
            );
        }

        const auto pageResult =
            repository->search(filter);

        const auto pageUrl =
            [&baseUrl](int page) {
                if (page <= 1) {
                    return baseUrl;
                }

                return baseUrl +
                    (baseUrl.find('?') ==
                             std::string::npos
                         ? "?"
                         : "&") +
                    "page=" +
                    std::to_string(page);
            };

        const auto returnUrl =
            pageUrl(pageResult.page);

        Json::Value projects(Json::arrayValue);

        for (const auto& project :
            pageResult.items) {
            Json::Value value;

            value["id"] = project.id;
            value["title"] = project.title;
            value["status"] = project.status;
            value["detailsUrl"] =
                "/projects/" +
                std::to_string(project.id) +
                "?returnUrl=" +
                drogular::Url::encode(returnUrl);

            projects.append(value);
        }

        Json::Value paginationPages(Json::arrayValue);

        for (int page = 1;
             page <= pageResult.totalPages;
             ++page) {
            Json::Value item(Json::objectValue);
            item["number"] = page;
            item["url"] = pageUrl(page);
            item["current"] =
                page == pageResult.page;

            paginationPages.append(
                std::move(item)
            );
        }

        context.set("projects", projects);
        context.set("hasProjects", !projects.empty());
        context.set("hasPagination", pageResult.totalPages > 1);
        context.set("paginationPages", paginationPages);
        context.set("currentPage", pageResult.page);
        context.set("totalPages", pageResult.totalPages);
        context.set("totalItems", pageResult.totalItems);
        context.set("hasPreviousPage", pageResult.page > 1);
        context.set(
            "previousPageUrl",
            pageUrl(pageResult.page - 1)
        );
        context.set(
            "hasNextPage",
            pageResult.page < pageResult.totalPages
        );
        context.set(
            "nextPageUrl",
            pageUrl(pageResult.page + 1)
        );
    }

    std::string templatePath() const override {
        return "projects.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};