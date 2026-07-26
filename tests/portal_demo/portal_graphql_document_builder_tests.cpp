#include "data/portal_schema.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "features/roles/graphql/documents/role_queries.hpp"
#include "providers/graphql/documents/project_type_queries.hpp"

#include <gtest/gtest.h>

TEST(PortalGraphQLDocumentBuilderTests, BuildsAllQuery) {
    const auto query =
        PortalGraphQLDocumentBuilder::all(
            "PortalRoles",
            "roles",
            PortalSchema::roles()
        );

    EXPECT_EQ(
        query.toString(),
        RoleQueries::all().toString()
    );
}

TEST(PortalGraphQLDocumentBuilderTests, BuildsFindByIdQuery) {
    const auto query =
        PortalGraphQLDocumentBuilder::findById(
            "PortalProjectTypeById",
            "projectType",
            PortalSchema::projectTypes()
        );

    EXPECT_EQ(
        query.toString(),
        ProjectTypeQueries::findById().toString()
    );
}

TEST(PortalGraphQLDocumentBuilderTests, BuildsFindByFieldQuery) {
    const auto query =
        PortalGraphQLDocumentBuilder::findByField(
            "PortalRoleByCode",
            "role",
            "code",
            "String!",
            PortalSchema::roles()
        );

    EXPECT_EQ(
        query.toString(),
        RoleQueries::findByCode().toString()
    );
}