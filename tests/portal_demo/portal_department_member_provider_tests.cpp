#include "features/department_members/providers/portal_memory_department_member_provider.hpp"

#include <gtest/gtest.h>

TEST(PortalDepartmentMemberProviderTests, ListsMembersByDepartment) {
    PortalMemoryDepartmentMemberProvider provider({
        {1, 10, 100},
        {2, 10, 101},
        {3, 11, 100}
    });

    const auto members =
        provider.membersOfDepartment(10);

    ASSERT_EQ(members.size(), 2u);
    EXPECT_EQ(members[0].userId, 100);
    EXPECT_EQ(members[1].userId, 101);
}

TEST(PortalDepartmentMemberProviderTests, ListsDepartmentsByUser) {
    PortalMemoryDepartmentMemberProvider provider({
        {1, 10, 100},
        {2, 11, 100},
        {3, 11, 101}
    });

    const auto memberships =
        provider.departmentsOfUser(100);

    ASSERT_EQ(memberships.size(), 2u);
    EXPECT_EQ(memberships[0].departmentId, 10);
    EXPECT_EQ(memberships[1].departmentId, 11);
}

TEST(PortalDepartmentMemberProviderTests, AddingDuplicateIsIdempotent) {
    PortalMemoryDepartmentMemberProvider provider({
        {
            7,
            10,
            100
        }
    });

    const auto value =
        provider.addMember(10, 100);

    EXPECT_EQ(value.id, 7);
    EXPECT_EQ(provider.membersOfDepartment(10).size(), 1u);
}

TEST(PortalDepartmentMemberProviderTests, AddsAndRemovesMembership) {
    PortalMemoryDepartmentMemberProvider provider;

    const auto value =
        provider.addMember(10, 100);

    EXPECT_GT(value.id, 0);
    EXPECT_TRUE(provider.find(10, 100).has_value());

    EXPECT_TRUE(provider.removeMember(10, 100));
    EXPECT_FALSE(provider.find(10, 100).has_value());
    EXPECT_FALSE(provider.removeMember(10, 100));
}
