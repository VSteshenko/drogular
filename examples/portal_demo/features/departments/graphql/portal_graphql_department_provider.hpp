#pragma once

#include "features/departments/providers/department_provider.hpp"
#include "features/departments/graphql/documents/department_queries.hpp"
#include "features/departments/graphql/documents/department_mutations.hpp"
#include "features/departments/graphql/mappers/department_mapper.hpp"
#include "core/portal_string_utils.hpp"

#include <drogular/graphql_client.hpp>

#include <memory>

class PortalGraphQLDepartmentProvider final : public PortalDepartmentProvider {
public:
    explicit PortalGraphQLDepartmentProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

    std::vector<PortalDepartment> all() const override {
        const auto response =
            client_->execute(
                DepartmentQueries::all()
            );

        const auto values =
            response.field("departments");

        return values
            ? DepartmentMapper::fromList(*values)
            : std::vector<PortalDepartment>{};
    }

    drogular::PagedResult<PortalDepartment> search(
        const PortalDepartmentQuery& query
    ) const override {
        const auto response =
            client_->execute(
                DepartmentQueries::search(query),
                DepartmentMapper::toVariables(query)
            );

        const auto value =
            response.field("departmentPage");

        return value
            ? DepartmentMapper::pageFromValue(*value)
            : drogular::PagedResult<PortalDepartment>{};
    }

    std::optional<PortalDepartment> findById(
        int id
    ) const override {
        const auto response =
            client_->execute(
                DepartmentQueries::findById(id),
                DepartmentMapper::idVariables(id)
            );

        return DepartmentMapper::optionalDepartment(response.field("department"));
    }

    bool exists(
        const std::string& name,
        std::optional<int> excludingId = std::nullopt
    ) const override {
        PortalDepartmentQuery query;
        query.search = name;
        query.pageSize = 100;

        for (const auto& item : search(query).items) {
            if ((!excludingId || item.id != *excludingId) &&
                portalAsciiLowercase(item.name) == portalAsciiLowercase(name)) {
                return true;
            }
        }

        return false;
    }

    PortalDepartment create(
        const PortalDepartmentCreate& input
    ) override {
        PortalDepartment value{
            0,
            input.name,
            input.description,
            input.managerId,
            input.isActive
        };

        const auto response =
            client_->execute(
                DepartmentMutations::create(value),
                DepartmentMapper::toVariables(value)
            );

        return DepartmentMapper::fromValue(response.data()["createDepartment"]);
    }

    PortalDepartment update(
        const PortalDepartmentUpdate& input
    ) override {
        const auto response =
            client_->execute(
                DepartmentMutations::update(input),
                DepartmentMapper::toVariables(input)
            );

        return DepartmentMapper::fromValue(response.data()["updateDepartment"]);
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};