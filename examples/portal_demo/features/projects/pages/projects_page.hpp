#pragma once

#include "features/projects/providers/project_provider.hpp"
#include "providers/project_type_provider.hpp"
#include "features/users/providers/user_provider.hpp"
#include "ui/portal_page_support.hpp"
#include "data/portal_schema.hpp"
#include "localization/portal_error_translator.hpp"
#include "features/projects/ui/portal_project_query_view_model.hpp"
#include "features/projects/ui/portal_project_query_parser.hpp"
#include "features/projects/ui/portal_project_query_serializer.hpp"

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

        const auto query =
            PortalProjectQueryParser::fromRequest(request);

        const auto search =
            query.search.value_or("");
        const auto status =
            query.status.value_or("");
        const auto projectTypeId =
            query.projectTypeId;
        const auto ownerId =
            query.ownerId;
        const auto sort =
            query.sorting.empty()
                ? PortalProjectSort{
                      .field = "title",
                      .direction =
                          PortalProjectSortDirection::Ascending
                  }
                : query.sorting.front();
        const auto& sortField = sort.field;
        const auto sortDirection = sort.direction;

        context.set("projectSearch", search);
        context.set("projectStatusFilter", status);

        Json::Value statusOptions(Json::arrayValue);

        const auto addStatusOption =
            [&statusOptions, &status](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] = status == value;

                statusOptions.append(std::move(option));
            };

        addStatusOption(
            "",
            context.translate(
                "projects.query.status.all"
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
            option["label"] = context.translate("projects.filter.type.all");
            option["selected"] = !projectTypeId.has_value();

            projectTypeFilterOptions.append(std::move(option));
        }

        for (const auto& type : allProjectTypes) {
            Json::Value option(Json::objectValue);

            option["value"] = type.id;
            option["label"] = type.title;
            option["selected"] =
                projectTypeId.has_value() &&
                *projectTypeId == type.id;

            projectTypeFilterOptions.append(std::move(option));
        }

        auto users =
            context.requireService<PortalUserProvider>();
        const auto allUsers =
            users->all();
        Json::Value ownerFilterOptions(Json::arrayValue);
        {
            Json::Value option(Json::objectValue);

            option["value"] = "";
            option["label"] = context.translate("projects.filter.owner.all");
            option["selected"] = !ownerId.has_value();

            ownerFilterOptions.append(std::move(option));
        }

        for (const auto& user : allUsers) {
            Json::Value option(Json::objectValue);

            option["value"] = user.id;
            option["label"] = user.username;
            option["selected"] =
                ownerId.has_value() &&
                *ownerId == user.id;

            ownerFilterOptions.append(std::move(option));
        }

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

                sortOptions.append(std::move(option));
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

        Json::Value directionOptions(Json::arrayValue);

        const auto selectedDirection = toString(sortDirection);
        const auto addDirectionOption =
            [&directionOptions, &selectedDirection](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] =
                    selectedDirection == value;

                directionOptions.append(std::move(option));
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

        portal::PortalProjectQueryViewModel filters;

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

        auto repository =
            context.requireService<PortalProjectProvider>();
        const auto pageResult =
            repository->search(query);

        const auto pageUrl =
            [&query](int page) {
                auto pageQuery = query;
                pageQuery.page = std::max(1, page);
                return std::string("/projects") +
                    PortalProjectQuerySerializer::toQueryString(pageQuery);
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