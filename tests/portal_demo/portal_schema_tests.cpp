#include "data/portal_schema.hpp"

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

TEST(PortalSchemaTests, UsesLabelKeyBeforeGeneratedKey) {
    auto schema =
        PortalTableSchema<PortalProject>::forModel("projects");

    schema
        .field("title", &PortalProject::title)
            .labelKey("custom.title");

    EXPECT_EQ(
        schema.fieldLabelKey(
            "title"
        ),
        "custom.title"
    );
}

TEST(PortalSchemaTests, UsesGeneratedKeyWhenNoLabelMetadataExists) {
    auto schema =
        PortalTableSchema<PortalProject>::forModel("projects");

    schema.field("title", &PortalProject::title);

    EXPECT_EQ(
        schema.fieldLabelKey(
            "title"
        ),
        "projects.title"
    );
}

TEST(PortalSchemaTests, BuildsDefaultFieldLabelKeys) {
    const auto schema =
        PortalSchema::projects();

    EXPECT_EQ(
        schema.fieldLabelKey("title"),
        "projects.title.label"
    );

    EXPECT_EQ(
        schema.fieldLabelKey("status"),
        "projects.status.label"
    );

    EXPECT_EQ(
        schema.fieldLabelKey("ownerId"),
        "projects.owner"
    );

    EXPECT_EQ(
        schema.fieldLabelKey("projectTypeId"),
        "projects.type"
    );
}

TEST(PortalSchemaTests, SupportsCustomFieldLabelKey) {
    auto schema =
        PortalTableSchema<PortalProject>::forModel(
            "projects"
        );

    schema
        .field("ownerId", &PortalProject::ownerId)
            .labelKey("projects.owner");

    EXPECT_EQ(
        schema.fieldLabelKey("ownerId"),
        "projects.owner"
    );
}

TEST(PortalSchemaTests, ProjectReferencesHaveDisplayFields) {
    const auto schema =
        PortalSchema::projects();

    const auto owner =
        schema.fieldByName("ownerId");

    ASSERT_NE(owner, nullptr);
    ASSERT_TRUE(owner->reference.has_value());

    EXPECT_EQ(owner->reference->table, "users");
    EXPECT_EQ(owner->reference->field, "id");
    EXPECT_EQ(owner->reference->displayField, "username");

    const auto type =
        schema.fieldByName("projectTypeId");

    ASSERT_NE(type, nullptr);
    ASSERT_TRUE(type->reference.has_value());

    EXPECT_EQ(type->reference->table, "projectTypes");
    EXPECT_EQ(type->reference->field, "id");
    EXPECT_EQ(type->reference->displayField, "title");
}