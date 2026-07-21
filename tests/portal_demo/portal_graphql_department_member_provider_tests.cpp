#include "features/department_members/graphql/portal_graphql_department_member_provider.hpp"

#include <drogular/static_graphql_client.hpp>

#include <gtest/gtest.h>

TEST(PortalGraphQLDepartmentMemberProviderTests, ReadsDepartmentMembers) {
    Json::Value values(Json::arrayValue);

    Json::Value item(Json::objectValue);
    item["id"] = 1;
    item["departmentId"] = 2;
    item["userId"] = 3;

    values.append(item);

    Json::Value data(Json::objectValue);
    data["departmentMembers"] = values;

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(data);

    PortalGraphQLDepartmentMemberProvider provider(client);

    const auto result =
        provider.membersOfDepartment(2);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].userId, 3);
    EXPECT_EQ(
        client->lastRequest()->variables()["departmentId"].asInt(),
        2
    );
}

TEST(PortalGraphQLDepartmentMemberProviderTests, AddsAndRemovesMember) {
    Json::Value added(Json::objectValue);
    added["id"] = 7;
    added["departmentId"] = 2;
    added["userId"] = 4;

    Json::Value addData(Json::objectValue);
    addData["addDepartmentMember"] = added;

    auto addClient =
        std::make_shared<drogular::StaticGraphQLClient>(addData);

    PortalGraphQLDepartmentMemberProvider addProvider(addClient);

    EXPECT_EQ(
        addProvider.addMember(2, 4).id,
        7
    );

    Json::Value removeData(Json::objectValue);
    removeData["removeDepartmentMember"] = true;

    auto removeClient =
        std::make_shared<drogular::StaticGraphQLClient>(removeData);

    PortalGraphQLDepartmentMemberProvider removeProvider(removeClient);

    EXPECT_TRUE(removeProvider.removeMember(2, 4));
}