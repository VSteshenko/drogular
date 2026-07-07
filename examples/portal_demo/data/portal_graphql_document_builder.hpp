#pragma once

#include "portal_graphql_selection_builder.hpp"
#include "portal_table_schema.hpp"

#include <drogular/graphql.hpp>

#include <string>
#include <utility>

class PortalGraphQLDocumentBuilder {
public:
    template <typename TModel>
    static drogular::gql::Query all(
        std::string operationName,
        std::string fieldName,
        const PortalTableSchema<TModel>& schema
    ) {
        return drogular::gql::query(
            std::move(operationName)
        )
            .select(
                drogular::gql::field(
                    std::move(fieldName)
                )
                    .children(
                        PortalGraphQLSelectionBuilder::from(
                            schema
                        )
                    )
            );
    }

    template <typename TModel>
    static drogular::gql::Query findById(
        std::string operationName,
        std::string fieldName,
        const PortalTableSchema<TModel>& schema
    ) {
        return findByField(
            std::move(operationName),
            std::move(fieldName),
            "id",
            "ID!",
            schema
        );
    }

    template <typename TModel>
    static drogular::gql::Query findByField(
        std::string operationName,
        std::string fieldName,
        std::string variableName,
        std::string variableType,
        const PortalTableSchema<TModel>& schema
    ) {
        return drogular::gql::query(
            std::move(operationName)
        )
            .variable(
                variableName,
                variableType
            )
            .select(
                drogular::gql::field(
                    std::move(fieldName)
                )
                    .arg(
                        variableName,
                        drogular::gql::variable(
                            variableName
                        )
                    )
                    .children(
                        PortalGraphQLSelectionBuilder::from(
                            schema
                        )
                    )
            );
    }

    template <typename TModel>
    static drogular::gql::Mutation mutation(
        std::string operationName,
        std::string fieldName,
        std::string variableName,
        std::string variableType,
        const PortalTableSchema<TModel>& schema
    ) {
        return drogular::gql::mutation(
                   std::move(operationName)
               )
               .variable(
                   variableName,
                   variableType
               )
               .select(
                   drogular::gql::field(
                       std::move(fieldName)
                   )
                   .arg(
                       variableName,
                       drogular::gql::variable(
                           variableName
                       )
                   )
                   .children(
                       PortalGraphQLSelectionBuilder::from(
                           schema
                       )
                   )
               );
    }
};