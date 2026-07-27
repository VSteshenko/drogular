#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "features/users/data/portal_user_sort.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"

#include <drogular/graphql_operation_registry.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class UserGraphQLOperations {
public:
    explicit UserGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(drogular::GraphQLOperationRegistry& registry) {
        // Queries
        registry.registerQuery(
            "PortalUsers",
            [this](const auto&) {
                return usersResponse();
            });
        registry.registerQuery(
            "PortalUserByCredentials",
            [this](const auto& variables) {
                return userByCredentialsResponse(
                    variables.json()["username"].asString(),
                    variables.json()["password"].asString()
                );
            });
        registry.registerQuery("SearchPortalUsers", [this](const auto& variables) {
            const auto& json = variables.json();
            const auto search = json.isMember("search")
                ? json["search"].asString()
                : std::string();
            const auto role = json.isMember("role")
                ? json["role"].asString()
                : std::string();

            std::vector<PortalUserSort> sorting;
            if (json.isMember("sorting") && json["sorting"].isArray()) {
                for (const auto& item : json["sorting"]) {
                    const auto field = item["field"].asString();
                    if (field != "id" && field != "username" && field != "role") {
                        continue;
                    }
                    sorting.push_back({
                        .field = field,
                        .direction = item["direction"].asString() == "desc"
                            ? PortalSortDirection::Descending
                            : PortalSortDirection::Ascending
                    });
                }
            }

            const auto page =
                json.isMember("page")
                    ? std::max(1, json["page"].asInt())
                    : 1;
            const auto pageSize =
                json.isMember("pageSize")
                    ? std::max(1, json["pageSize"].asInt())
                    : 10;

            return searchUsersResponse(search, role, sorting, page, pageSize);
        });
        // Mutations
        registry.registerMutation(
            "CreatePortalUser",
            [this](const auto& variables) {
                return createUserResponse(variables.json()["user"]);
            });
        registry.registerMutation(
            "UpdatePortalUser",
            [this](const auto& variables) {
                return updateUserResponse(variables.json()["user"]);
            });
    }

private:
    static Json::Value userJson(
        const PortalUser& user
    ) {
        return PortalSchemaMapper::toJson(
            PortalSchema::users(),
            user
        );
    }

    drogular::GraphQLResponse usersResponse() const {
        Json::Value users(Json::arrayValue);

        for (const auto& user : dataset_->users()) {
            users.append(userJson(user));
        }

        Json::Value data(Json::objectValue);
        data["users"] = users;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse userByCredentialsResponse(
        const std::string& username,
        const std::string& password
    ) const {
        Json::Value data(Json::objectValue);
        data["userByCredentials"] = Json::Value();

        for (const auto& user : dataset_->users()) {
            if (user.username == username &&
                user.password == password) {
                data["userByCredentials"] =
                    userJson(user);
                break;
                }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    int nextUserId() const {
        int nextId = 1;

        for (const auto& user : dataset_->users()) {
            nextId = std::max(nextId, user.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createUserResponse(
        const Json::Value& value
    ) {
        auto user =
            PortalSchemaMapper::fromJson(
                PortalSchema::users(),
                value
            );

        user.id =
            nextUserId();

        dataset_->users().push_back(user);

        Json::Value data(Json::objectValue);
        data["createUser"] = userJson(user);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateUserResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateUser"] = Json::Value();

        const auto id =
            value["id"].asInt();

        for (auto& user : dataset_->users()) {
            if (user.id != id) {
                continue;
            }

            if (value.isMember("username")) {
                user.username =
                    value["username"].asString();
            }

            if (value.isMember("role")) {
                user.role =
                    value["role"].asString();
            }

            data["updateUser"] =
                userJson(user);

            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse searchUsersResponse(
        const std::string& search,
        const std::string& role,
        std::vector<PortalUserSort> sorting,
        int page,
        int pageSize
    ) const {
        std::vector<PortalUser> result;
        const auto needle = PortalGraphQLOperationSupport::lowercase(search);

        for (const auto& user : dataset_->users()) {
            if (!needle.empty() &&
                PortalGraphQLOperationSupport::lowercase(user.username).find(needle) ==
                    std::string::npos) {
                continue;
            }
            if (!role.empty() && user.role != role) {
                continue;
            }
            result.push_back(user);
        }

        if (sorting.empty()) {
            sorting.push_back({
                .field = "username",
                .direction = PortalSortDirection::Ascending
            });
        }

        const auto compare = []<typename T>(
            const T& left,
            const T& right,
            PortalSortDirection direction
        ) {
            return direction == PortalSortDirection::Ascending
                ? left < right
                : right < left;
        };

        std::stable_sort(
            result.begin(),
            result.end(),
            [&sorting, &compare](
                const PortalUser& left,
                const PortalUser& right
            ) {
                for (const auto& sort : sorting) {
                    if (sort.field == "username" &&
                        left.username != right.username) {
                        return compare(
                            left.username,
                            right.username,
                            sort.direction
                        );
                    }
                    if (sort.field == "role" &&
                        left.role != right.role) {
                        return compare(
                            left.role,
                            right.role,
                            sort.direction
                        );
                    }
                    if (sort.field == "id" &&
                        left.id != right.id) {
                        return compare(
                            left.id,
                            right.id,
                            sort.direction
                        );
                    }
                }
                return left.id < right.id;
            }
        );

        const auto totalItems = static_cast<int>(result.size());
        const auto totalPages = std::max(
            1,
            (totalItems + pageSize - 1) / pageSize
        );
        const auto beginIndex =
            static_cast<std::size_t>(page - 1) *
            static_cast<std::size_t>(pageSize);
        const auto endIndex = std::min(
            result.size(),
            beginIndex + static_cast<std::size_t>(pageSize)
        );

        Json::Value items(Json::arrayValue);
        if (beginIndex < result.size()) {
            for (auto index = beginIndex;
                 index < endIndex;
                 ++index) {
                items.append(userJson(result[index]));
            }
        }

        Json::Value userPage(Json::objectValue);
        userPage["items"] = std::move(items);
        userPage["page"] = page;
        userPage["pageSize"] = pageSize;
        userPage["totalItems"] = totalItems;
        userPage["totalPages"] = totalPages;

        Json::Value data(Json::objectValue);
        data["userPage"] = std::move(userPage);
        return PortalGraphQLOperationSupport::response(data);
    }

    std::shared_ptr<PortalDataset> dataset_;
};