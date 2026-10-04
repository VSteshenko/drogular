#pragma once

#include "features/users/providers/user_provider.hpp"
#include "features/users/ui/portal_user_query_parser.hpp"
#include "features/users/ui/portal_user_query_serializer.hpp"
#include "features/roles/providers/role_provider.hpp"

#include <drogular/pagination_model.hpp>
#include <drogular/render_context.hpp>
#include <drogular/url.hpp>

#include <algorithm>

class PortalUsersBrowserSupport final {
public:
    static void apply(drogular::RenderContext& context) {
        const auto query =
            PortalUserQueryParser::fromRequest(context.request());
        const auto search = query.search.value_or("");
        const auto role = query.role.value_or("");
        const auto sort = query.sorting.empty()
            ? PortalUserSort{
                  .field = "username",
                  .direction = PortalSortDirection::Ascending
              }
            : query.sorting.front();

        context.set("userSearch", search);

        auto roles = context.requireService<PortalRoleProvider>();
        const auto allRoles = roles->all();

        Json::Value roleFilterOptions(Json::arrayValue);
        {
            Json::Value option(Json::objectValue);
            option["value"] = "";
            option["labelKey"] = "users.filter.role.all";
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
        context.set("userRoleFilterOptions", roleFilterOptions);

        Json::Value sortOptions(Json::arrayValue);
        const auto addSortOption =
            [&sortOptions, &sort](
                const std::string& value,
                const std::string& labelKey
            ) {
                Json::Value option(Json::objectValue);
                option["value"] = value;
                option["labelKey"] = labelKey;
                option["selected"] = sort.field == value;
                sortOptions.append(std::move(option));
            };
        addSortOption("username", "users.sort.username");
        addSortOption("role", "users.sort.role");
        addSortOption("id", "users.sort.id");
        context.set("userSortOptions", sortOptions);

        Json::Value directionOptions(Json::arrayValue);
        const auto selectedDirection = toString(sort.direction);
        const auto addDirection =
            [&directionOptions, &selectedDirection](
                const std::string& value,
                const std::string& labelKey
            ) {
                Json::Value option(Json::objectValue);
                option["value"] = value;
                option["labelKey"] = labelKey;
                option["selected"] = selectedDirection == value;
                directionOptions.append(std::move(option));
            };
        addDirection("asc", "users.sort.ascending");
        addDirection("desc", "users.sort.descending");
        context.set("userSortDirectionOptions", directionOptions);

        auto repository = context.requireService<PortalUserProvider>();
        const auto pageResult = repository->search(query);

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
                "/edit?returnUrl=" + drogular::Url::encode(returnUrl);
            users.append(std::move(value));
        }

        context.set("users", users);
        context.set("userTotalItems", pageResult.totalItems);
        context.setJson(
            "pagination",
            drogular::makePaginationModel(
                pageResult.page,
                pageResult.totalPages,
                pageUrl
            )
        );
    }
};