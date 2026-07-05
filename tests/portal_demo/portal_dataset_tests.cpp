#include "../../examples/portal_demo/data/demo_dataset.hpp"

#include <gtest/gtest.h>

TEST(PortalDatasetTests, ValidatesValidDataset)
{
    const auto dataset =
        DemoDataset::create();

    EXPECT_TRUE(
        dataset.validate().valid()
    );
}

TEST(PortalDatasetTests, DetectsDuplicateUserIds)
{
    PortalDataset dataset;

    dataset
        .addUser({.id = 1, .username = "admin", .password = "x", .role = "admin"})
        .addUser({.id = 1, .username = "user", .password = "x", .role = "user"});

    const auto validation =
        dataset.validate();

    EXPECT_FALSE(validation.valid());
}

TEST(PortalDatasetTests, DetectsDuplicateRoleCodes)
{
    PortalDataset dataset;

    dataset
        .addRole({.id = 1, .code = "admin", .title = "Administrator"})
        .addRole({.id = 2, .code = "admin", .title = "Admin Copy"});

    EXPECT_FALSE(
        dataset.validate().valid()
    );
}

TEST(PortalDatasetTests, DetectsMissingProjectOwner)
{
    PortalDataset dataset;

    dataset.addProject({
        .id = 1,
        .title = "Orphan Project",
        .status = "active",
        .ownerId = 99
    });

    EXPECT_FALSE(
        dataset.validate().valid()
    );
}