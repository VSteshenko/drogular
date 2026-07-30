#include <drogular/query_string_builder.hpp>

#include <gtest/gtest.h>

TEST(QueryStringBuilderTests, BuildsEncodedQueryString) {
    const auto query = drogular::QueryStringBuilder{}
        .add("search", "hello world")
        .add("page", 2)
        .build();

    EXPECT_EQ(
        query,
        "?search=hello%20world&page=2"
    );
}

TEST(QueryStringBuilderTests, SkipsConditionalValues) {
    const auto query = drogular::QueryStringBuilder{}
        .addIf(false, "search", "ignored")
        .addIf(true, "page", 3)
        .build();

    EXPECT_EQ(
        query,
        "?page=3");
}

TEST(QueryStringBuilderTests, IsEmptyBeforeValuesAreAdded) {
    const drogular::QueryStringBuilder builder;

    EXPECT_TRUE(builder.empty());
    EXPECT_TRUE(builder.build().empty());
}

TEST(QueryStringBuilderTests, PreservesExplicitEmptyValues) {
    const auto query = drogular::QueryStringBuilder{}
        .add("search", "")
        .build();

    EXPECT_EQ(
        query,
        "?search="
    );
}

TEST(QueryStringBuilderTests, SkipsEmptyStringValuesExplicitly) {
    const auto query = drogular::QueryStringBuilder{}
        .addNonEmpty("search", "")
        .addNonEmpty("role", "admin")
        .build();

    EXPECT_EQ(
        query,
        "?role=admin"
    );
}

TEST(QueryStringBuilderTests, AddsOnlyNonEmptyOptionalStrings) {
    const std::optional<std::string> missing;
    const std::optional<std::string> empty = std::string{};
    const std::optional<std::string> value = "active";

    const auto query = drogular::QueryStringBuilder{}
        .add("missing", missing)
        .add("empty", empty)
        .add("status", value)
        .build();

    EXPECT_EQ(
        query,
        "?status=active"
    );
}