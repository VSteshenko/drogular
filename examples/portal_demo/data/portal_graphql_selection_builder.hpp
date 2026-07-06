#pragma once

#include "portal_table_schema.hpp"

#include <drogular/graphql.hpp>

#include <vector>

class PortalGraphQLSelectionBuilder {
public:
    template <typename TModel>
    static std::vector<drogular::gql::Selection> from(
        const PortalTableSchema<TModel>& schema
    ) {
        std::vector<drogular::gql::Selection> selections;

        for (const auto& field : schema.fields()) {
            selections.push_back(
                drogular::gql::field(field.name)
            );
        }

        return selections;
    }
};