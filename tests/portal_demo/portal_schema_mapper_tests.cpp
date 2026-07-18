#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "data/models/portal_user.hpp"

#include <gtest/gtest.h>

TEST(PortalSchemaMapperTests, MapsUserToJson) {
    PortalUser user;
    user.id = 1;
    user.username = "admin";
    user.password = "secret";
    user.role = "admin";

    const auto json =
        PortalSchemaMapper::toJson(
            PortalSchema::users(),
            user
        );

    EXPECT_EQ(json["id"].asInt(), 1);
    EXPECT_EQ(json["username"].asString(), "admin");
    EXPECT_EQ(json["password"].asString(), "secret");
    EXPECT_EQ(json["role"].asString(), "admin");
}

TEST(PortalSchemaMapperTests, MapsJsonToProject) {
    Json::Value json(Json::objectValue);

    json["id"] = 1;
    json["title"] = "Portal";
    json["status"] = "active";
    json["ownerId"] = 2;
    json["projectTypeId"] = 3;

    const auto project =
        PortalSchemaMapper::fromJson(
            PortalSchema::projects(),
            json
        );

    EXPECT_EQ(project.id, 1);
    EXPECT_EQ(project.title, "Portal");
    EXPECT_EQ(project.status, "active");
    EXPECT_EQ(project.ownerId, 2);
    EXPECT_EQ(project.projectTypeId, 3);
}

TEST(PortalSchemaMapperTests, MapsJsonToUser) {
    Json::Value json(Json::objectValue);

    json["id"] = 1;
    json["username"] = "admin";
    json["password"] = "secret";
    json["role"] = "admin";

    const auto user =
        PortalSchemaMapper::fromJson(
            PortalSchema::users(),
            json
        );

    EXPECT_EQ(user.id, 1);
    EXPECT_EQ(user.username, "admin");
    EXPECT_EQ(user.password, "secret");
    EXPECT_EQ(user.role, "admin");
}