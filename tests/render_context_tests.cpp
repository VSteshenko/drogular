#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(RenderContextTests, StoresAndReadsStringValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.get<std::string>("title");

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, "Hello");
}

TEST(RenderContextTests, StoresAndReadsIntegerValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.get<int>("count");

    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 42);
}

TEST(RenderContextTests, ReturnsNulloptForMissingKey) {
    drogular::RenderContext context;

    const auto value = context.get<std::string>("missing");

    EXPECT_FALSE(value.has_value());
}

TEST(RenderContextTests, ReturnsNulloptForWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto value = context.get<std::string>("count");

    EXPECT_FALSE(value.has_value());
}

TEST(RenderContextTests, ChecksContains) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    EXPECT_TRUE(context.contains("title"));
    EXPECT_FALSE(context.contains("missing"));
}

TEST(RenderContextTests, RemovesValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.remove("title");

    EXPECT_FALSE(context.contains("title"));
}

TEST(RenderContextTests, ClearsValues) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.set("count", 42);

    context.clear();

    EXPECT_FALSE(context.contains("title"));
    EXPECT_FALSE(context.contains("count"));
}

TEST(RenderContextTests, RequiresExistingValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.require<std::string>("title");

    EXPECT_EQ(title, "Hello");
}

TEST(RenderContextTests, ThrowsWhenRequiredValueIsMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.require<std::string>("title"),
        drogular::RenderContextError
    );
}

TEST(RenderContextTests, ThrowsWhenRequiredValueHasWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    EXPECT_THROW(
        context.require<std::string>("count"),
        drogular::RenderContextError
    );
}

TEST(RenderContextTests, ReturnsDefaultValueWhenKeyIsMissing) {
    drogular::RenderContext context;

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 0);
}

TEST(RenderContextTests, ReturnsStoredValueInsteadOfDefaultValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 42);
}

TEST(RenderContextTests, ChildContextReadsParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();

    EXPECT_EQ(child.require<std::string>("title"), "Parent");
}

TEST(RenderContextTests, ChildContextOverridesParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();
    child.set("title", std::string("Child"));

    EXPECT_EQ(child.require<std::string>("title"), "Child");
    EXPECT_EQ(parent.require<std::string>("title"), "Parent");
}

TEST(RenderContextTests, ChildContextContainsParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();

    EXPECT_TRUE(child.contains("title"));
}