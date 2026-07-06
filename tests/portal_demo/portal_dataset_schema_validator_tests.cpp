#include "../../examples/portal_demo/data/demo_dataset.hpp"
#include "../../examples/portal_demo/data/portal_dataset.hpp"
#include "../../examples/portal_demo/data/portal_dataset_schema_validator.hpp"

#include <gtest/gtest.h>

TEST(PortalDatasetSchemaValidatorTests, ValidatesDemoDataset)
{
    const auto dataset =
        DemoDataset::create();

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_TRUE(validation.valid());
}

TEST(PortalDatasetSchemaValidatorTests, DetectsDuplicateUserId)
{
    PortalDataset dataset;

    dataset
        .addRole({.id = 1, .code = "admin", .title = "Administrator"})
        .addUser({.id = 1, .username = "admin", .password = "x", .role = "admin"})
        .addUser({.id = 1, .username = "other", .password = "x", .role = "admin"});

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_FALSE(validation.valid());
}

TEST(PortalDatasetSchemaValidatorTests, DetectsMissingRoleReference)
{
    PortalDataset dataset;

    dataset.addUser({
        .id = 1,
        .username = "admin",
        .password = "x",
        .role = "missing"
    });

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_FALSE(validation.valid());
}

TEST(PortalDatasetSchemaValidatorTests, DetectsMissingProjectOwnerReference)
{
    PortalDataset dataset;

    dataset
        .addProjectType({.id = 1, .code = "customer", .title = "Customer"})
        .addProject({
            .id = 1,
            .title = "Project",
            .status = "active",
            .ownerId = 99,
            .projectTypeId = 1
        });

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_FALSE(validation.valid());
}

TEST(PortalDatasetSchemaValidatorTests, DetectsMissingProjectTypeReference)
{
    PortalDataset dataset;

    dataset
        .addUser({.id = 1, .username = "admin", .password = "x", .role = "admin"})
        .addRole({.id = 1, .code = "admin", .title = "Administrator"})
        .addProject({
            .id = 1,
            .title = "Project",
            .status = "active",
            .ownerId = 1,
            .projectTypeId = 99
        });

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_FALSE(validation.valid());
}
