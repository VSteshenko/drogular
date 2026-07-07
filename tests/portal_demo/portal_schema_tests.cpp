#include "../../examples/portal_demo/data/portal_schema.hpp"

#include <gtest/gtest.h>

TEST(PortalSchemaTests, DefinesUserSchema) {
    const auto schema =
        PortalSchema::users();

    EXPECT_EQ(schema.name(), "users");
    ASSERT_EQ(schema.fields().size(), 4);

    EXPECT_EQ(schema.fields()[0].name, "id");
    EXPECT_TRUE(schema.fields()[0].key);
    EXPECT_TRUE(schema.fields()[0].unique);
    EXPECT_TRUE(schema.fields()[0].required);

    EXPECT_EQ(schema.fields()[3].name, "role");
    ASSERT_TRUE(schema.fields()[3].reference.has_value());
    EXPECT_EQ(schema.fields()[3].reference->table, "roles");
    EXPECT_EQ(schema.fields()[3].reference->field, "code");
}

TEST(PortalSchemaTests, ProjectReferencesHaveDisplayFields) {
    const auto schema =
        PortalSchema::projects();

    const auto& owner =
        schema.fields()[3];

    ASSERT_TRUE(owner.reference.has_value());
    EXPECT_EQ(owner.reference->table, "users");
    EXPECT_EQ(owner.reference->field, "id");
    EXPECT_EQ(owner.reference->displayField, "username");
    EXPECT_EQ(owner.displayName, "Owner");

    const auto& type =
        schema.fields()[4];

    ASSERT_TRUE(type.reference.has_value());
    EXPECT_EQ(type.reference->table, "projectTypes");
    EXPECT_EQ(type.reference->field, "id");
    EXPECT_EQ(type.reference->displayField, "title");
    EXPECT_EQ(type.displayName, "Type");
}