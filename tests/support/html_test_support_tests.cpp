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

TEST(HtmlTestSupportTests, ReturnsElementContentByMarker) {
    const std::string html =
        R"(
<select id="status">
    <option value="active">Active</option>
</select>
)";

    const auto element =
        HtmlTestSupport::elementContent(
            html,
            R"(id="status")",
            "</select>"
        );

    EXPECT_NE(
        element.find(
            R"(<select id="status">)"
        ),
        std::string::npos
    );

    EXPECT_NE(
        element.find(
            R"(value="active")"
        ),
        std::string::npos
    );

    EXPECT_NE(
        element.find(
            "</select>"
        ),
        std::string::npos
    );
}

TEST(HtmlTestSupportTests, ReturnsEmptyElementContentWhenMarkerIsMissing) {
    const std::string html =
        R"(<select id="status"></select>)";

    EXPECT_TRUE(
        HtmlTestSupport::elementContent(
            html,
            R"(id="missing")",
            "</select>"
        ).empty()
    );
}

TEST(HtmlTestSupportTests, DetectsSelectedOptionInsideSpecificSelect) {
    const std::string html =
        R"(
<select id="createStatus">
    <option value="active">Active</option>
</select>

<select id="filterStatus">
    <option value="active" selected>Active</option>
</select>
)";

    EXPECT_TRUE(
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(id="filterStatus")",
            "active"
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(id="createStatus")",
            "active"
        )
    );
}

TEST(HtmlTestSupportTests, ReturnsFalseWhenSelectedOptionSelectIsMissing) {
    const std::string html =
        R"(<select id="status"></select>)";

    EXPECT_FALSE(
        HtmlTestSupport::optionSelectedInSelect(
            html,
            R"(id="missing")",
            "active"
        )
    );
}

TEST(HtmlTestSupportTests, DecodesHtmlEntities) {
    EXPECT_EQ(
        HtmlTestSupport::decodeEntities(
            "/projects?search=port&amp;status=active"
        ),
        "/projects?search=port&status=active"
    );
}

TEST(HtmlTestSupportTests, MatchesAttributesOnTheSameElementRegardlessOfOrder) {
    const std::string html =
        R"(
<input value="a" class="dg-input" name="search">
<input name="page" value="2">
)";

    EXPECT_TRUE(
        HtmlTestSupport::elementHasAttributes(
            html,
            "input",
            {
                {"name", "search"},
                {"value", "a"}
            }
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::elementHasAttributes(
            html,
            "input",
            {
                {"name", "search"},
                {"value", "2"}
            }
        )
    );
}

TEST(HtmlTestSupportTests, MatchesBooleanAttributeInsideSpecificParent) {
    const std::string html =
        R"(
<select name="sort">
    <option value="title" selected>Title</option>
    <option value="id">ID</option>
</select>
<select name="direction">
    <option value="asc">Ascending</option>
    <option value="desc" selected>Descending</option>
</select>
)";

    EXPECT_TRUE(
        HtmlTestSupport::elementContainsElementWithAttributes(
            html,
            "select",
            {{"name", "direction"}},
            "option",
            {
                {"value", "desc"},
                {"selected", ""}
            }
        )
    );

    EXPECT_FALSE(
        HtmlTestSupport::elementContainsElementWithAttributes(
            html,
            "select",
            {{"name", "sort"}},
            "option",
            {
                {"value", "desc"},
                {"selected", ""}
            }
        )
    );
}

TEST(HtmlTestSupportTests, EmptyExpectedValueMatchesExplicitEmptyAttribute) {
    const std::string html =
        R"(<input name="search" dg-reset-value="">)";

    EXPECT_TRUE(
        HtmlTestSupport::elementHasAttributes(
            html,
            "input",
            {
                {"name", "search"},
                {"dg-reset-value", ""}
            }
        )
    );
}