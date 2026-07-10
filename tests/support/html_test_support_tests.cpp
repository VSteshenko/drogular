#include "html_test_support.hpp"

#include <gtest/gtest.h>

TEST(HtmlTestSupportTests, FindsOpeningTagByMarker) {
    const std::string html =
        R"(<input id="title" name="title" required>)";

    EXPECT_EQ(
        HtmlTestSupport::openingTag(
            html,
            R"(name="title")"
        ),
        R"(<input id="title" name="title" required>)"
    );
}

TEST(HtmlTestSupportTests, DetectsAttribute) {
    const std::string html =
        R"(<select name="role" required></select>)";

    EXPECT_TRUE(
        HtmlTestSupport::hasAttribute(
            html,
            R"(name="role")",
            "required"
        )
    );
}

TEST(HtmlTestSupportTests, ReadsAttributeValue) {
    const std::string html =
        R"(<option value="admin" selected>Admin</option>)";

    const auto value =
        HtmlTestSupport::attributeValue(
            html,
            R"(value="admin")",
            "value"
        );

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "admin");
}