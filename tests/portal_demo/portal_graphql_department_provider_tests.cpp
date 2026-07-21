#include "features/departments/graphql/portal_graphql_department_provider.hpp"

#include <drogular/static_graphql_client.hpp>

#include <gtest/gtest.h>

TEST(PortalGraphQLDepartmentProviderTests, ReadsAndFindsDepartments) {
    Json::Value departments(Json::arrayValue);

    Json::Value item(Json::objectValue);
    item["id"] = 1;
    item["name"] = "Engineering";
    item["description"] = "Platform";
    item["managerId"] = 1;
    item["isActive"] = true;

    departments.append(item);

    Json::Value data(Json::objectValue);
    data["departments"] = departments;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLDepartmentProvider provider(client);

    const auto result = provider.all();

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].name, "Engineering");
}

TEST(PortalGraphQLDepartmentProviderTests, CreatesDepartment) {
    Json::Value item(Json::objectValue);
    item["id"] = 4;
    item["name"] = "Sales";
    item["description"] = "Sales team";
    item["managerId"] = 2;
    item["isActive"] = true;

    Json::Value data(Json::objectValue);
    data["createDepartment"] = item;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLDepartmentProvider provider(client);

    PortalDepartmentCreate input{
        "Sales",
        "Sales team",
        2,
        true
    };

    const auto result = provider.create(input);

    EXPECT_EQ(result.id, 4);
    EXPECT_EQ(
        client->lastRequest()->variables()["department"]["name"].asString(),
        "Sales"
    );
}