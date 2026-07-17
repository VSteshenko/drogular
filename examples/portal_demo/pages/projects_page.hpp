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

        auto repository =
            context.requireService<PortalProjectProvider>();

        Json::Value projects(Json::arrayValue);

        PortalProjectFilter filter;
        std::string returnUrl = "/projects";
        std::string separator = "?";

        if (!search.empty()) {
            filter.search = search;

            returnUrl +=
                separator +
                "search=" +
                drogular::Url::encode(search);
            separator = "&";
        }

        if (!status.empty()) {
            filter.status = status;

            returnUrl +=
                separator +
                "status=" +
                drogular::Url::encode(status);
            separator = "&";
        }

        if (projectTypeId.has_value()) {
            filter.projectTypeId = *projectTypeId;

            returnUrl +=
                separator +
                "projectTypeId=" +
                std::to_string(*projectTypeId);
            separator = "&";
        }

        if (ownerId.has_value()) {
            filter.ownerId =
                *ownerId;

            returnUrl +=
                separator +
                "ownerId=" +
                std::to_string(*ownerId);
            separator = "&";
        }

        filter.sorting.push_back({
            .field = sortField,
            .direction = sortDirection
        });

        const auto hasCustomSorting =
            sortField != "title" ||
            sortDirection !=
                PortalProjectSortDirection::
                    Ascending;

        if (hasCustomSorting) {
            returnUrl +=
                separator +
                "sort=" +
                drogular::Url::encode(
                    sortField
                );
            separator = "&";

            returnUrl +=
                separator +
                "direction=" +
                toString(sortDirection);
        }

        for (const auto& project :
            repository->search(filter)) {
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

        context.set("projects", projects);
        context.set("hasProjects", !projects.empty());
    }

    std::string templatePath() const override {
        return "projects.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};