#pragma once


#include "features/users/providers/user_provider.hpp"
#include "features/users/ui/portal_user_query_parser.hpp"
#include "features/users/ui/portal_user_query_serializer.hpp"
#include "features/roles/providers/role_provider.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "data/portal_schema.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>
#include <drogular/pagination_model.hpp>

#include <algorithm>

class PortalUsersPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(context, "users.title");

        const auto request = context.request();
        const auto error = request != nullptr
            ? request->getParameter("error")
            : std::string();
        const auto success = request != nullptr
            ? request->getParameter("success")
            : std::string();
        const auto username = request != nullptr
            ? request->getParameter("username")
            : std::string();

        const auto usersError =
            PortalErrorTranslator::usersError(context, error);
        const auto usersSuccess =
            PortalErrorTranslator::usersSuccess(context, success);

        context.set("createUsername", username);
        context.set("hasUsersError", !usersError.empty());
        context.set("usersError", usersError);
        context.set("hasUsersSuccess", !usersSuccess.empty());
        context.set("usersSuccess", usersSuccess);
        context.set(
            "alertMessage",
            !usersError.empty() ? usersError : usersSuccess
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto schema = PortalSchema::users();
        context.set(
            "usersUsername",
            context.translate(schema.fieldLabelKey("username"))
        );
        context.set(
            "usersPassword",
            context.translate(schema.fieldLabelKey("password"))
        );
        context.set(
            "usersRole",
            context.translate(schema.fieldLabelKey("role"))
        );
        context.set("usersRoleRequired", schema.fieldRequired("role"));

        auto roles =
            context.requireService<PortalRoleProvider>();
        const auto allRoles =
            roles->all();

        Json::Value roleOptions(Json::arrayValue);

        for (const auto& role : allRoles) {
            Json::Value option(Json::objectValue);

            option["value"] = role.code;
            option["label"] = role.title;
            option["selected"] = false;

            roleOptions.append(
                std::move(option)
            );
        }

        context.set(
            "roleOptions",
            roleOptions
        );

        const auto query =
            PortalUserQueryParser::fromRequest(request);
        const auto search =
            query.search.value_or("");
        const auto role =
            query.role.value_or("");
        const auto sort =
            query.sorting.empty()
            ? PortalUserSort{
                  .field = "username",
                  .direction = PortalSortDirection::Ascending
              }
            : query.sorting.front();

        context.set("userSearch", search);

        Json::Value roleFilterOptions(Json::arrayValue);
        {
            Json::Value option(Json::objectValue);

            option["value"] = "";
            option["label"] = context.translate("users.filter.role.all");
            option["selected"] = role.empty();

            roleFilterOptions.append(std::move(option));
        }
        for (const auto& item : allRoles) {
            Json::Value option(Json::objectValue);

            option["value"] = item.code;
            option["label"] = item.title;
            option["selected"] = role == item.code;

            roleFilterOptions.append(std::move(option));
        }

        context.set(
            "userRoleFilterOptions",
            roleFilterOptions
        );

        Json::Value sortOptions(Json::arrayValue);
        const auto addSortOption =
            [&sortOptions, &sort](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);

                option["value"] = value;
                option["label"] = label;
                option["selected"] = sort.field == value;

                sortOptions.append(std::move(option));
            };
        addSortOption(
            "username",
            context.translate(
                "users.sort.username"
            )
        );
        addSortOption(
            "role",
            context.translate(
                "users.sort.role"
            )
        );
        addSortOption(
            "id",
            context.translate(
                "users.sort.id"
            )
        );
        context.set(
            "userSortOptions",
            sortOptions
        );

        Json::Value directionOptions(Json::arrayValue);

        const auto selectedDirection = toString(sort.direction);
        const auto addDirection =
            [&directionOptions, &selectedDirection](
                const std::string& value,
                const std::string& label
            ) {
                Json::Value option(Json::objectValue);
                option["value"] = value;
                option["label"] = label;
                option["selected"] = selectedDirection == value;
                directionOptions.append(std::move(option));
            };
        addDirection(
            "asc",
            context.translate(
                "users.sort.ascending"
            )
        );
        addDirection(
            "desc",
            context.translate(
                "users.sort.descending"
            )
        );
        context.set(
            "userSortDirectionOptions",
            directionOptions
        );
        context.set(
            "hasActiveUserFilters",
            !search.empty() || !role.empty()
        );

        auto repository =
            context.requireService<
                PortalUserProvider
            >();
        const auto pageResult =
            repository->search(query);

        const auto pageUrl = [&query](int page) {
            auto pageQuery = query;
            pageQuery.page = std::max(1, page);
            return std::string("/users") +
                PortalUserQuerySerializer::toQueryString(pageQuery);
        };
        const auto returnUrl = pageUrl(pageResult.page);

        Json::Value users(Json::arrayValue);
        for (const auto& user : pageResult.items) {
            Json::Value value(Json::objectValue);

            value["id"] = user.id;
            value["username"] = user.username;
            value["role"] = user.role;
            value["editUrl"] =
                "/users/" + std::to_string(user.id) +
                "/edit?returnUrl=" +
                drogular::Url::encode(returnUrl);

            users.append(std::move(value));
        }

        const auto pagination = drogular::makePaginationModel(
            pageResult.page,
            pageResult.totalPages,
            pageUrl
        );

        context.set("users", users);
        context.set("hasUsers", !users.empty());
        context.setJson("pagination", pagination);
        context.set("userTotalItems", pageResult.totalItems);
    }

    std::string templatePath() const override {
        return "users.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};