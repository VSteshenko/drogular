#include "../../examples/portal_demo/data/portal_schema.hpp"
#include "../../examples/portal_demo/data/portal_schema_mapper.hpp"
#include "../../examples/portal_demo/portal_user.hpp"

#include <gtest/gtest.h>

TEST(PortalSchemaMapperTests, MapsUserToJson)
{
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