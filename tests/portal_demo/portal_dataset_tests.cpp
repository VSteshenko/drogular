#include "../../examples/portal_demo/data/demo_dataset.hpp"

#include <gtest/gtest.h>

TEST(PortalDatasetTests, ValidatesValidDataset) {
    const auto dataset =
        DemoDataset::create();

    EXPECT_TRUE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}

TEST(PortalDatasetTests, DetectsDuplicateUserIds) {
    PortalDataset dataset;

    dataset
        .addUser({.id = 1, .username = "admin", .password = "x", .role = "admin"})
        .addUser({.id = 1, .username = "user", .password = "x", .role = "user"});

    const auto validation =
        PortalDatasetSchemaValidator::validate(dataset);

    EXPECT_FALSE(validation.valid());
}

TEST(PortalDatasetTests, DetectsDuplicateRoleCodes) {
    PortalDataset dataset;

    dataset
        .addRole({.id = 1, .code = "admin", .title = "Administrator"})
        .addRole({.id = 2, .code = "admin", .title = "Admin Copy"});

    EXPECT_FALSE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}

TEST(PortalDatasetTests, DetectsMissingProjectOwner) {
    PortalDataset dataset;

    dataset.addProject({
        .id = 1,
        .title = "Orphan Project",
        .status = "active",
        .ownerId = 99
    });

    EXPECT_FALSE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}

TEST(PortalDatasetTests, DetectsDuplicateProjectTypeIds) {
    PortalDataset dataset;

    dataset
        .addProjectType({.id = 1, .code = "customer", .title = "Customer"})
        .addProjectType({.id = 1, .code = "internal", .title = "Internal"});

    EXPECT_FALSE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}

TEST(PortalDatasetTests, DetectsDuplicateProjectTypeCodes) {
    PortalDataset dataset;

    dataset
        .addProjectType({.id = 1, .code = "customer", .title = "Customer"})
        .addProjectType({.id = 2, .code = "customer", .title = "Copy"});

    EXPECT_FALSE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}

TEST(PortalDatasetTests, DetectsMissingProjectType) {
    PortalDataset dataset;

    dataset
        .addUser({.id = 1, .username = "admin", .password = "x", .role = "admin"})
        .addProject({
            .id = 1,
            .title = "Project",
            .status = "active",
            .ownerId = 1,
            .projectTypeId = 99
        });

    EXPECT_FALSE(
        PortalDatasetSchemaValidator::validate(dataset).valid()
    );
}