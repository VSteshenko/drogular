#include "features/departments/providers/portal_memory_department_provider.hpp"
#include "features/departments/ui/portal_department_query_serializer.hpp"

#include <gtest/gtest.h>

TEST(PortalDepartmentQueryTests, DefaultsReturnAllSortedByName) {
    PortalMemoryDepartmentProvider provider({
        {2,"Zeta","",1,true},
        {1,"Alpha","",2,false}
    });

    const auto page =
        provider.search({});

    ASSERT_EQ(page.totalItems, 2);
    ASSERT_EQ(page.items.size(), 2);
    EXPECT_EQ(page.items[0].name, "Alpha");
}

TEST(PortalDepartmentQueryTests, FiltersSearchAndActiveState) {
    PortalMemoryDepartmentProvider provider({
        {1,"Engineering","Platform",1,true},
        {2,"Archive","Historical",1,false}
    });

    PortalDepartmentQuery query;
    query.search = "plat";
    query.isActive = true;

    const auto page =
        provider.search(query);

    ASSERT_EQ(page.items.size(), 1);
    EXPECT_EQ(page.items[0].name, "Engineering");
}

TEST(PortalDepartmentQueryTests, SerializesUrlState) {
    PortalDepartmentQuery query;
    query.search = "Research & Development";
    query.isActive = false;
    query.sorting.push_back(
    {"managerId", PortalDepartmentSortDirection::Descending}
    );
    query.page = 2;

    const auto value =
        PortalDepartmentQuerySerializer::toQueryString(query);

    EXPECT_NE(value.find("search=Research%20%26%20Development"), std::string::npos);
    EXPECT_NE(value.find("active=false"), std::string::npos);
    EXPECT_NE(value.find("page=2"), std::string::npos);
}