#include "department_queries.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query DepartmentQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalDepartments",
        "departments",
        PortalSchema::departments()
    );
}

drogular::gql::Query DepartmentQueries::search(const PortalDepartmentQuery&) {
    return drogular::gql::query("SearchPortalDepartments")
        .variable("search", "String")
        .variable("isActive", "Boolean")
        .variable("sorting", "[DepartmentSortInput!]")
        .variable("page", "Int!")
        .variable("pageSize", "Int!")
        .select(
            drogular::gql::field("departmentPage")
                .arg("search", drogular::gql::variable("search"))
                .arg("isActive", drogular::gql::variable("isActive"))
                .arg("sorting", drogular::gql::variable("sorting"))
                .arg("page", drogular::gql::variable("page"))
                .arg("pageSize", drogular::gql::variable("pageSize"))
                .children({
                    drogular::gql::field("items").children(
                        PortalGraphQLSelectionBuilder::from(PortalSchema::departments())
                    ),
                    drogular::gql::field("page"),
                    drogular::gql::field("pageSize"),
                    drogular::gql::field("totalItems"),
                    drogular::gql::field("totalPages")
                })
        );
}

drogular::gql::Query DepartmentQueries::findById(int) {
    return drogular::gql::query("PortalDepartmentById")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("department")
                .arg("id", drogular::gql::variable("id"))
                .children(PortalGraphQLSelectionBuilder::from(PortalSchema::departments()))
        );
}