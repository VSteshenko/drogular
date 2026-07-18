#include "data/portal_graphql_selection_builder.hpp"
#include "data/portal_schema.hpp"

#include <drogular/static_graphql_client.hpp>

#include <gtest/gtest.h>

TEST(PortalGraphQLSelectionBuilderTests, BuildsSelectionsFromProjectSchema) {
    const auto selections =
        PortalGraphQLSelectionBuilder::from(
            PortalSchema::projects()
        );

    ASSERT_EQ(selections.size(), 5);

    EXPECT_EQ(selections[0].name(), "id");
    EXPECT_EQ(selections[1].name(), "title");
    EXPECT_EQ(selections[2].name(), "status");
    EXPECT_EQ(selections[3].name(), "ownerId");
    EXPECT_EQ(selections[4].name(), "projectTypeId");
}