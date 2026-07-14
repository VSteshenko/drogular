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